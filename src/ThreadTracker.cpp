/*****************************************************
			 PROJECT  : numaprof
			 VERSION  : 2.3.0
			 DATE     : 05/2017
			 AUTHOR   : Valat Sébastien - CERN
			 LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <cassert>
#include "ThreadTracker.hpp"
#include "../extern-deps/from-numactl/MovePages.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
ThreadTracker::ThreadTracker(ProcessTracker * process)
              :allocTracker(process->getPageTable())
{
	assert(process != NULL);
	this->process = process;
	this->numa = process->getNumaAffinity();
	this->table = process->getPageTable();
	printf("Numa initial mapping : %d\n",numa);
}

/*******************  FUNCTION  *********************/
void ThreadTracker::flush(void)
{
	this->process->mergeInstruction(instructions);
	this->allocTracker.flush(process);
}

/*******************  FUNCTION  *********************/
void ThreadTracker::onSetAffinity(cpu_set_t * mask)
{
	this->numa = process->getNumaAffinity(mask);
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
			page.numaNode = pageNode;
	}

	//get instr
	Stats & instr = instructions[ip];

	//get malloc relation
	MallocInfos * allocInfos = (MallocInfos *)page.getAllocPointer(addr);
	Stats * allocStats;
	if (allocInfos == NULL)
		allocStats = &dummyAlloc;
	else
		allocStats = allocInfos->stats;

	//cases
	if (pageNode == NUMAPROF_DEFAULT_NUMA_NODE)
	{
		//check unpinned first access
		if (numa == -1)
		{
			stats.unpinnedFirstTouch++;
			instr.unpinnedFirstTouch++;
			allocStats->unpinnedFirstTouch++;
		} else {
			stats.firstTouch++;
			instr.firstTouch++;
			allocStats->firstTouch++;
		}

		//if write, consider that we create the page so
		//check if we are pinned to remember status for latter access
		if (write)
			page.fromPinnedThread = (numa != -1);
	} else {
		//if thread is not pinned
		if (numa == -1)
		{
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

	if (instructions.size() >= 1000)
	{
		this->process->mergeInstruction(instructions);
		instructions.clear();
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
	table->clear(addr,size);
}

/*******************  FUNCTION  *********************/
void ThreadTracker::onAlloc(size_t ip,size_t ptr,size_t size)
{
	//printf("%lu => %p => %lu\n",ip,(void*)ptr,size);
	allocTracker.onAlloc(ip,ptr,size);
}

/*******************  FUNCTION  *********************/
void ThreadTracker::onFree(size_t ptr)
{
	//printf("free %p\n",(void*)ptr);
	allocTracker.onFree(ptr);
}

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const ThreadTracker& value)
{
	json.openStruct();
		json.printField("stats",value.stats);
		json.printField("numa",value.numa);
	json.closeStruct();
}

}
