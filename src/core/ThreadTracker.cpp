/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <sys/mman.h>
#include <cassert>
#include "ThreadTracker.hpp"
#include "../common/Debug.hpp"
#include "../../extern-deps/from-numactl/MovePages.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
ThreadTracker::ThreadTracker(ProcessTracker * process)
              :allocTracker(process->getPageTable())
              ,accessMatrix(process->getNumaTopo().getNumaNodes())
{
	assert(process != NULL);
	this->process = process;
	this->numa = process->getNumaAffinity(&cpuBindList);
	this->table = process->getPageTable();
	this->topo = &process->getNumaTopo();
	this->bindingLog.push_back(this->numa);
	printf("Numa initial mapping : %d\n",numa);
}

/*******************  FUNCTION  *********************/
void ThreadTracker::flush(void)
{
	this->process->mergeInstruction(instructions);

	//flush to keep smell
	for (AllocCacheMap::iterator it = allocCache.begin() ; it != allocCache.end() ; ++it)
		*(it->first) = it->second;
	allocCache.clear();

	this->allocTracker.flush(process);
}

/*******************  FUNCTION  *********************/
void ThreadTracker::onSetAffinity(cpu_set_t * mask)
{
	this->numa = process->getNumaAffinity(mask,&cpuBindList);
	this->bindingLog.push_back(this->numa);
}

/*******************  FUNCTION  *********************/
void ThreadTracker::onAccess(size_t ip,size_t addr,bool write)
{
	//printf("Access %p => %p\n",(void*)ip,(void*)addr);
	//get numa location of page form page table
	Page & page = table->getPage(addr);

	//extract
	int pageNode = page.numaNode;

	//if not defined use move pages
	if (pageNode == NUMAPROF_DEFAULT_NUMA_NODE)
	{
		pageNode = getNumaOfPage(addr);
		//printf("Page on %d, write = %d\n",pageNode,write);
		if (write && pageNode != NUMAPROF_DEFAULT_NUMA_NODE)
		{
			//if (table->canBeHugePage(addr))
			//	table->setHugePageNuma(addr,pageNode);
			//else
			page.numaNode = pageNode;
			#ifdef NUMAPROF_HUGE_CHECK
				if (page.canBeHugePage)
					if (pageNode != getNumaOfPage(addr & (~NUMAPROG_HUGE_PAGE_MASK)))
						numaprofWarning("Expect huge page but get different mapping inside !");
			#endif

			process->onAfterFirstTouch(pageNode);
		}
	} 
	
	//extrct mini stack
	#ifdef NUMAPROG_CALLSTACK
		MiniStack miniStack;
		stack.fillMiniStack(miniStack);
	#endif

	//get instr
	#ifdef NUMAPROG_CALLSTACK
		Stats & instr = instructions[miniStack];
	#else
		Stats & instr = instructions[ip];
	#endif
	
	//acces matrix
	if (pageNode >= 0)
		accessMatrix.access(numa,pageNode);

	//get malloc relation
	MallocInfos * allocInfos = (MallocInfos *)page.getAllocPointer(addr);
	Stats * allocStats;
	if (allocInfos == NULL)
	{
		allocStats = &dummyAlloc;
	} else {
		allocStats = allocInfos->stats;
		//pick from alloc cache, this is a track to keep data local to thread
		//and not use atomics/locks everywhere. Hence it make good scalability
		//on large number of cores and NUMA nodes.
		allocStats = &(allocCache[allocStats]);
	}

	//cases
	if (pageNode <= NUMAPROF_DEFAULT_NUMA_NODE)
	{
		size_t touchedPages = 1;
		//if write, consider that we create the page so
		//check if we are pinned to remember status for latter access
		if (write)
		{
			if (page.canBeHugePage) {
				//nothing to do, already done
				//CAUTION it rely on the fact that table->canBeHugePage() is called ONLY HERE.
			} else if (table->canBeHugePage(addr)) {
				table->setHugePageFromPinnedThread(addr,numa != -1);
				touchedPages = NUMAPROG_HUGE_PAGE_SIZE / NUMAPROF_PAGE_SIZE;
			} else { 
				page.fromPinnedThread = (numa != -1);
			}
		}
		
		//check unpinned first access
		if (numa == -1)
		{
			stats.unpinnedFirstTouch += touchedPages;
			instr.unpinnedFirstTouch += touchedPages;
			allocStats->unpinnedFirstTouch += touchedPages;
		} else {
			stats.firstTouch += touchedPages;
			instr.firstTouch += touchedPages;
			allocStats->firstTouch += touchedPages;
		}
	} else {
		assert(pageNode >= 0);
		if (topo->getIsMcdram(pageNode)) 
		{
			stats.mcdramAccess++;
			instr.mcdramAccess++;
			allocStats->mcdramAccess++;
		} else if (numa == -1) {
			//check if page came from pin thread or not
			if (page.fromPinnedThread)
			{
				stats.unpinnedThreadAccess++;
				instr.unpinnedThreadAccess++;
				allocStats->unpinnedThreadAccess++;
			} else {
				stats.unpinnedBothAccess++;
				instr.unpinnedBothAccess++;
				allocStats->unpinnedBothAccess++;
			}
		} else {
			//check if page came from pin thread or not
			if (page.fromPinnedThread == false)
			{
				stats.unpinnedPageAccess++;
				instr.unpinnedPageAccess++;
				allocStats->unpinnedPageAccess++;
			} else if (numa == pageNode) {
				//if local
				stats.localAccess++;
				instr.localAccess++;
				allocStats->localAccess++;
			} else {
				//if remote
				stats.remoteAccess++;
				instr.remoteAccess++;
				allocStats->remoteAccess++;
			}
		}
	}

	//flush to keep small
	if (instructions.size() >= 200)
	{
		this->process->mergeInstruction(instructions);
		instructions.clear();
	}

	//flush to keep smell
	if (allocCache.size() >= 200)
	{
		for (AllocCacheMap::iterator it = allocCache.begin() ; it != allocCache.end() ; ++it)
			*(it->first) = it->second;
		allocCache.clear();
	}
}

/*******************  FUNCTION  *********************/
void ThreadTracker::onStop(void)
{
	this->process->mergeInstruction(instructions);
	instructions.clear();
}

/*******************  FUNCTION  *********************/
void ThreadTracker::onMunmap(size_t addr,size_t size)
{
	process->onMunmap(addr,size);
}

/*******************  FUNCTION  *********************/
void ThreadTracker::onMmap(size_t addr,size_t size,size_t flags,size_t fd)
{
	if (flags == MAP_ANON)
		fd = NUMAPROF_PAGE_ANON_FD;
	table->trackMMap(addr,size,fd);
}

/*******************  FUNCTION  *********************/
void ThreadTracker::onMremap(size_t oldAddr,size_t oldSize,size_t newAddr, size_t newSize)
{
	table->mremap(oldAddr,oldSize,newAddr,newSize);
}

/*******************  FUNCTION  *********************/
void ThreadTracker::onAlloc(size_t ip,size_t ptr,size_t size)
{
	//printf("%lu => %p => %lu\n",ip,(void*)ptr,size);
	#ifdef NUMAPROG_CALLSTACK
		MiniStack miniStack;
		stack.fillMiniStack(miniStack);
		allocTracker.onAlloc(miniStack,ptr,size);
	#else
		allocTracker.onAlloc(ip,ptr,size);
	#endif
}

/*******************  FUNCTION  *********************/
void ThreadTracker::onRealloc(size_t ip, size_t oldPtr, size_t newPtr, size_t newSize)
{
	#ifdef NUMAPROG_CALLSTACK
		MiniStack miniStack;
		stack.fillMiniStack(miniStack);
		allocTracker.onFree(oldPtr);
		allocTracker.onAlloc(miniStack,newPtr,newSize);
	#else
		allocTracker.onFree(oldPtr);
		allocTracker.onAlloc(ip,newPtr,newSize);
	#endif
}

/*******************  FUNCTION  *********************/
void ThreadTracker::onFree(size_t ptr)
{
	//printf("free %p\n",(void*)ptr);
	allocTracker.onFree(ptr);
}

/*******************  FUNCTION  *********************/
void ThreadTracker::onEnterFunction(void * addr)
{
	stack.push(addr);
}

/*******************  FUNCTION  *********************/
void ThreadTracker::onExitFunction(void)
{
	stack.pop();
}

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const ThreadTracker& value)
{
	json.openStruct();
		json.printField("stats",value.stats);
		json.printField("numa",value.numa);
		json.printField("binding",value.cpuBindList);
		json.printField("accessMatrix",value.accessMatrix);
		json.printField("bindingLog",value.bindingLog);
	json.closeStruct();
}

}
