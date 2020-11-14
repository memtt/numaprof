/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.1
             DATE     : 11/2020
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <sys/mman.h>
#include <cassert>
#include "ThreadTracker.hpp"
#include "../common/Debug.hpp"
#include "../portability/OS.hpp"
#include "../common/Options.hpp"
#include "../caches/CpuCacheBuilder.hpp"
#include "linux/mempolicy.h"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
/**
 * Constructor of a thread tracker. We should create one for each thread and store
 * it into a TLS.
 * @param process Define the global process tracker to sync the metrics and access 
 * the NUMA topology of the machine.
**/
ThreadTracker::ThreadTracker(ProcessTracker * process)
              :allocTracker(process->getPageTable())
              ,accessMatrix(process->getNumaTopo().getNumaNodes())
{
	assert(process != NULL);
	this->process = process;
	this->numa = process->getNumaAffinity(&cpuBindList);
	this->table = process->getPageTable();
	this->topo = &process->getNumaTopo();
	this->clockStart = Clock::get();
	this->tid = OS::getTID();
	this->mbindCalls = 0;
	this->memPolicy = topo->getCurrentMemPolicy();
	this->instructionFlush = 0;
	this->allocFlush = 0;
	this->cacheEntries = getGlobalOptions().coreThreadCacheEntries;
	logBinding(memPolicy);
	logBinding(this->numa);
	
	//build access batch
	this->accessBatch.reserve(getGlobalOptions().coreAccessBatchSize);
	
	//allocate and initial
	this->distanceCnt = new size_t[topo->getDistanceMax()+2];
	memset(distanceCnt,0,sizeof(size_t)*(topo->getDistanceMax()+2));
	
	//check verbosity
	if (!getGlobalOptions().outputSilent)
	{
		fprintf(stderr,"NUMAPROF: Numa initial mapping : %d\n",numa);
		fprintf(stderr,"NUMAPROF: Numa initial mem mapping : %s\n",getMemBindTypeName(memPolicy.type));
	}
	
	//build NUMA infos
	int numaNodes = topo->getNumaNodes();
	this->cntTouchedPages = new size_t[numaNodes];
	for (int i = 0 ; i < numaNodes ; i++)
		this->cntTouchedPages[i] = 0;

	//build cache
	std::string cacheType = getGlobalOptions().cacheType;
	this->cpuCache = CpuCacheBuilder::buildCache(cacheType,process->getCpuCacheLayout());
	this->cpuCache->onThreadMove(cpuBindList);
	this->hasCpuCache = !(cacheType == "dummy");
}

/*******************  FUNCTION  *********************/
/**
 * Return the ID of the thread.
**/
int ThreadTracker::getTID(void)
{
	return this->tid;
}

/*******************  FUNCTION  *********************/
/**
 * Add an entry to the binding log.
 * @param policy Define the new memory policy to log.
**/
void ThreadTracker::logBinding(MemPolicy & policy)
{
	ThreadMemBindingLogEntry entry;
	entry.at = Clock::get();
	entry.policy = policy;
	memPolicyLog.push_back(entry);
}

/*******************  FUNCTION  *********************/
/**
 * Add an entry to the binding log.
 * @parma numa Define the new NUMA binding to log.
**/
void ThreadTracker::logBinding(int numa)
{
	int last = -3;
	if (this->bindingLog.empty() == false)
		last = this->bindingLog.back().numa;
	if (numa != last)
	{
		ThreadBindingLogEntry entry;
		entry.at = Clock::get();
		entry.numa = numa;
		this->bindingLog.push_back(entry);
	}
}

/*******************  FUNCTION  *********************/
/**
 * Flush all the allocation statistics stored into local cache. It merge
 * all of them into the process storage and free the cache.
**/
void ThreadTracker::flushAllocCache(void)
{
	//flush to keep smell
	for (AllocCacheMap::iterator it = allocCache.begin() ; it != allocCache.end() ; ++it)
		(it->first)->merge(it->second);
	allocCache.clear();
}

/*******************  FUNCTION  *********************/
/**
 * To be used at thread exit, flush all the data from the thread.
**/
void ThreadTracker::flush(void)
{
	assert(allocCache.empty());
	flushAccessBatch();
	this->process->mergeInstruction(instructions);
	this->allocTracker.flush(process);
}

/*******************  FUNCTION  *********************/
/**
 * Hook to be called when the thread call sched_setaffinity to move the thread
 * on the topology.
 * @param mask Define the CPU mask to be used to extarct the NUMA binding.
 * @param size Define the size of the CPU mask.
**/
void ThreadTracker::onSetAffinity(cpu_set_t * mask,int size)
{
	flushAccessBatch();
	bindingLogMutex.lock();
	this->numa = process->getNumaAffinity(mask,size,&cpuBindList);
	this->logBinding(this->numa);
	bindingLogMutex.unlock();
	cpuCache->onThreadMove(cpuBindList);
}

/*******************  FUNCTION  *********************/
/**
 * Hook to be called when the thread call set_mempolicy.
 * @param mode Define the new mode to use.
 * @param mask Define the new mask
 * @param maxNodes Defineht the maximum number of NUMA node to consider (size of mask).
**/
void ThreadTracker::onSetMemPolicy(int mode,const unsigned long * mask,unsigned long maxNodes)
{
	flushAccessBatch();
	memPolicy.mode = mode;
	if (maxNodes % 8 != 0)
		maxNodes += 8 - maxNodes%8;
	size_t cnt = maxNodes / 8;
	if (cnt > 4)
			cnt = 4;
	for (unsigned long i = 0 ; i < cnt ; i++)
		memPolicy.mask[i] = mask[i];
	topo->staticComputeBindType(memPolicy);
	logBinding(memPolicy);
}

/*******************  FUNCTION  *********************/
/**
 * Check if the thread is assigned to a uniq NUMA node in therm of movement or
 * memory policy. Return true if bound to a uniq NUMA node, false otherwise.
**/
inline bool ThreadTracker::isMemBind(void)
{
	return (numa != -1 || memPolicy.type != MEMBIND_NO_BIND);
}

/*******************  FUNCTION  *********************/
/**
 * When using batch the thread should call this function to flush the access
 * batch.
**/
void ThreadTracker::flushAccessBatch()
{
	//skip
	if (accessBatch.capacity() == 0)
		return;
	
	//flush all
	for (size_t i = 0 ; i < accessBatch.size() ; i++)
	{
		AccessEvent & access = accessBatch[i];
		onAccessHandling(access.ip,access.addr,access.write,access.skip);
	}
	
	//clear
	accessBatch.clear();
}

/*******************  FUNCTION  *********************/
/**
 * Hook to be called by the instrumenter on memory accesses. The function will
 * build a batch is enabled otherwise it will directly handle the access.
 * @param ip Defnie the instruction pointer making the access.
 * @param addr Define the memory address which is accessed.
 * @param write True if it is a write, false if a read.
 * @param skip Define if we just handle the first touch and not update the counters.
**/
void ThreadTracker::onAccess(size_t ip,size_t addr,bool write,bool skip)
{
	//no batch
	if (accessBatch.capacity() == 0)
	{
		onAccessHandling(ip,addr,write,skip);
		return;
	}
	
	//add
	AccessEvent access = {ip,addr,write,skip};
	accessBatch.push_back(access);
	
	//flush if full
	if (accessBatch.size() == accessBatch.capacity())
		flushAccessBatch();
}

/*******************  FUNCTION  *********************/
/**
 * Function to be called by onAccess or batch flush to really apply the memory
 * accesses.
 * @param ip Defnie the instruction pointer making the access.
 * @param addr Define the memory address which is accessed.
 * @param write True if it is a write, false if a read.
 * @param skip Define if we just handle the first touch and not update the counters.
**/
void ThreadTracker::onAccessHandling(size_t ip,size_t addr,bool write,bool skip)
{
	//printf("Access %p => %p\n",(void*)ip,(void*)addr);

	//check cache, if contain data, just ignore
	if (hasCpuCache)
		if (this->cpuCache->onMemoryAccess(addr))
			return;

	//first check in TLB
	Page * page = tlb.get(addr >> NUMAPROF_PAGE_OFFSET);

	//if not in TLB get numa location of page form page table
	if (page == NULL)
	{
		//load
		page = &(table->getPage(addr));
		//store for next time
		tlb.set(addr >> NUMAPROF_PAGE_OFFSET, page);
	}

	//extract
	int pageNode = page->numaNode;
	bool isWriteFirstTouch = false;
	size_t touchedPages = 1;

	//if not defined use move pages
	if (pageNode <= NUMAPROF_DEFAULT_NUMA_NODE)
	{
		pageNode = OS::getNumaOfPage(addr);
		//printf("Page on %d, write = %d\n",pageNode,write);
		if (write && pageNode <= NUMAPROF_DEFAULT_NUMA_NODE)
		{
			isWriteFirstTouch = true;
			
			//touch
			//do first touch to ask where is the page
			//we use atomic to not modify the content in case this is not real first touch
			//so cannot write 0, just add 0
			__sync_fetch_and_add((char*)addr,0);
			
			//check
			if (page->canBeHugePage) {
				//this sould not append
				assert(false);
			} else if (table->canBeHugePage(addr)) {
				bool isHugePage;
				pageNode = OS::getNumaOfHugePage(addr,&isHugePage);
				assert(pageNode >= 0);
				if (isHugePage)
				{
					//mark as hue page and mark pinned status
					table->setHugePageFromPinnedThread(addr,pageNode,isMemBind());
					touchedPages = NUMAPROG_HUGE_PAGE_SIZE / NUMAPROF_PAGE_SIZE;
				} else {
					page->fromPinnedThread = isMemBind();
					page->numaNode = pageNode;
				}
			} else {
				page->numaNode = pageNode;
				page->fromPinnedThread = isMemBind();
			}
		}
		
		if (pageNode >= 0)
			page->numaNode = pageNode;

		if (pageNode >= 0)
			process->onAfterFirstTouch(pageNode,touchedPages);
	} 
	
	//if we skip we take care of the first touch (previous lines) but we do not account
	if (skip)
		return;
	
	//extrct mini stack
	#ifdef NUMAPROF_CALLSTACK
		MiniStack miniStack;
		stack.fillMiniStack(miniStack);
	#endif

	//get instr
	#ifdef NUMAPROF_CALLSTACK
		Stats * instr = &instructions[miniStack];
	#else
		Stats * instr = icache.get(ip);
		if (instr == NULL)
		{
			instr = &instructions[ip];
			icache.set(ip,instr);
		}
	#endif
	
	//acces matrix
	if (pageNode >= 0)
	{
		accessMatrix.access(numa,pageNode);
		distanceCnt[topo->getDistance(numa,pageNode)+1]++;
	}

	//get malloc relation
	MallocInfos * allocInfos = (MallocInfos *)(page->getAllocPointer(addr));
	Stats * allocStats;
	if (allocInfos == NULL)
	{
		allocStats = &dummyAlloc;
		stats.nonAlloc++;
		instr->nonAlloc++;
	} else {
		allocStats = allocInfos->stats;
		//pick from alloc cache, this is a track to keep data local to thread
		//and not use atomics/locks everywhere. Hence it make good scalability
		//on large number of cores and NUMA nodes.
		allocStats = acache.get((size_t)allocStats);
		if (allocStats == NULL)
		{
			allocStats = &(allocCache[allocInfos->stats]);
			acache.set((size_t)allocInfos->stats,allocStats);
		}
	}

	//cases
	if (pageNode <= NUMAPROF_DEFAULT_NUMA_NODE || isWriteFirstTouch)
	{
		//cound first touch pages
		if (pageNode >= 0)
			this->cntTouchedPages[pageNode]++;
		
		//check unpinned first access
		if (isMemBind() == false)
		{
			stats.unpinnedFirstTouch += touchedPages;
			instr->unpinnedFirstTouch += touchedPages;
			allocStats->unpinnedFirstTouch += touchedPages;
		} else {
			stats.firstTouch += touchedPages;
			instr->firstTouch += touchedPages;
			allocStats->firstTouch += touchedPages;
		}
	} else {
		assert(pageNode >= 0);
		if (topo->getIsMcdram(pageNode)) 
		{
			if (topo->getParentNode(pageNode) == numa)
			{
				stats.localMcdramAccess++;
				instr->localMcdramAccess++;
				allocStats->localMcdramAccess++;
			} else {
				stats.remoteMcdramAccess++;
				instr->remoteMcdramAccess++;
				allocStats->remoteMcdramAccess++;
			}
		} else if (numa == -1) {
			//check if page came from pin thread or not
			if (page->fromPinnedThread)
			{
				stats.unpinnedThreadAccess++;
				instr->unpinnedThreadAccess++;
				allocStats->unpinnedThreadAccess++;
			} else {
				stats.unpinnedBothAccess++;
				instr->unpinnedBothAccess++;
				allocStats->unpinnedBothAccess++;
			}
		} else {
			//check if page came from pin thread or not
			if (page->fromPinnedThread == false)
			{
				stats.unpinnedPageAccess++;
				instr->unpinnedPageAccess++;
				allocStats->unpinnedPageAccess++;
			} else if (numa == pageNode) {
				//if local
				stats.localAccess++;
				instr->localAccess++;
				allocStats->localAccess++;
			} else {
				//if remote
				stats.remoteAccess++;
				instr->remoteAccess++;
				allocStats->remoteAccess++;
			}
		}
	}

	//flush to keep small
	if (instructions.size() >= cacheEntries)
	{
		if (instructionFlush++ == 10000)
			fprintf(stderr,"NUMAPROF: Caution, flushing instruction a lot of time, maybe you need to increase flush threshold, current is %ld, see core:threadCacheSize !\n",cacheEntries);
		this->process->mergeInstruction(instructions);
		instructions.clear();
		icache.flush();
	}

	//flush to keep smell
	if (allocCache.size() >= cacheEntries)
	{
		if (allocFlush++ == 10000)
			fprintf(stderr,"NUMAPROF: Caution, flushing allocs a lot of time, maybe you need to increase flush threshold, current is %ld, see core:threadCacheSize !\n",cacheEntries);
		for (AllocCacheMap::iterator it = allocCache.begin() ; it != allocCache.end() ; ++it)
			(it->first)->merge(it->second);
		allocCache.clear();
		acache.flush();
	}
}

/*******************  FUNCTION  *********************/
/**
 * Hook to be called when the thread stop.
**/
void ThreadTracker::onStop(void)
{
	flushAccessBatch();
	this->process->mergeInstruction(instructions);
	instructions.clear();
	this->clockEnd = Clock::get();
}

/*******************  FUNCTION  *********************/
/**
 * Hook to be called when the thread call munmap. It transmit the operation
 * to the process handler.
 * @param addr Define the concerned address.
 * @param size Define the size of the selected segement.
**/
void ThreadTracker::onMunmap(size_t addr,size_t size)
{
	flushAccessBatch();
	process->onMunmap(addr,size);
}

/*******************  FUNCTION  *********************/
/**
 * Hook to be called when then thread call mmap.
**/
void ThreadTracker::onMmap(size_t addr,size_t size,size_t flags,size_t fd)
{
	flushAccessBatch();
	if (flags == MAP_ANON)
		fd = NUMAPROF_PAGE_ANON_FD;
	table->trackMMap(addr,size,fd);
}

/*******************  FUNCTION  *********************/
/**
 * Hook to be called when the thread call mremap.
**/
void ThreadTracker::onMremap(size_t oldAddr,size_t oldSize,size_t newAddr, size_t newSize)
{
	flushAccessBatch();
	table->mremap(oldAddr,oldSize,newAddr,newSize);
}

/*******************  FUNCTION  *********************/
/**
 * Hook to be called by the instrumenter when the thread call malloc.
**/
void ThreadTracker::onAlloc(size_t ip,size_t ptr,size_t size)
{
	flushAccessBatch();
	//printf("%lu => %p => %lu\n",ip,(void*)ptr,size);
	#ifdef NUMAPROF_CALLSTACK
		MiniStack miniStack;
		stack.fillMiniStack(miniStack);
		allocTracker.onAlloc(miniStack,ptr,size);
	#else
		allocTracker.onAlloc(ip,ptr,size);
	#endif
}

/*******************  FUNCTION  *********************/
/**
 * Hook to be called by the instrumenter when the thread call realloc.
**/
void ThreadTracker::onRealloc(size_t ip, size_t oldPtr, size_t newPtr, size_t newSize)
{
	flushAccessBatch();
	#ifdef NUMAPROF_CALLSTACK
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
/**
 * Hook to be called by the instrumenter when the thread call free.
**/
void ThreadTracker::onFree(size_t ptr)
{
	flushAccessBatch();
	//printf("free %p\n",(void*)ptr);
	allocTracker.onFree(ptr);
}

/*******************  FUNCTION  *********************/
/**
 * Hook to be called by the instrumenter when the thread enter in a function.
**/
void ThreadTracker::onEnterFunction(void * addr)
{
	stack.push(addr);
}

/*******************  FUNCTION  *********************/
/**
 * Hook to be called by the instrumenter when the thread exit a function.
**/
void ThreadTracker::onExitFunction(void)
{
	stack.pop();
}

/*******************  FUNCTION  *********************/
/**
 * Hook to be called when the thread call mbind to setup memory binding on segements.
**/
void ThreadTracker::onMBind(void * addr,size_t len,size_t mode,const unsigned long *nodemask,size_t maxnode,size_t flags)
{
	//flusha access batch
	flushAccessBatch();
	
	//basic capture
	mbindCalls++;
	
	//extarct policy
	MemPolicy policy;
	policy.mode = mode;
	if (maxnode % 8 != 0)
		maxnode += 8 - maxnode%8;
	size_t cnt = maxnode / 8;
	if (cnt > 4)
			cnt = 4;
	for (unsigned long i = 0 ; i < cnt ; i++)
		policy.mask[i] = nodemask[i];
	topo->staticComputeBindType(policy);
	
	//extract binding 
	bool pinned = (policy.type != MEMBIND_NO_BIND);
	
	//update page table
	table->onMbind(addr,len,pinned);
}

/*******************  FUNCTION  *********************/
/**
 * Dump the thread informations in a json format.
**/
void convertToJson(htopml::JsonState& json, const ThreadTracker& value)
{
	json.openStruct();
		json.printField("stats",value.stats);
		json.printField("numa",value.numa);
		json.printField("memPolicy",value.memPolicy);
		json.printField("binding",value.cpuBindList);
		json.printField("accessMatrix",value.accessMatrix);
		json.printFieldArray("distanceCnt",value.distanceCnt,value.topo->getDistanceMax()+2);
		json.printField("bindingLog",value.bindingLog);
		json.printField("memPolicyLog",value.memPolicyLog);
		json.printField("clockStart",value.clockStart);
		json.printField("clockEnd",value.clockEnd);
		json.printField("mbindCalls",value.mbindCalls);
		json.printFieldArray("touchedPages",value.cntTouchedPages,value.topo->getNumaNodes());
	json.closeStruct();
	
	#ifdef NUMAPROF_CACHE_STATS
		value.tlb.printStats("tlb");
		value.icache.printStats("icache");
		value.acache.printStats("acache");
	#endif
}

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const ThreadBindingLogEntry& value)
{
	json.openStruct();
		json.printField("at",value.at);
		json.printField("numa",value.numa);
	json.closeStruct();
}

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const ThreadMemBindingLogEntry& value)
{
	json.openStruct();
		json.printField("at",value.at);
		json.printField("policy",value.policy);
	json.closeStruct();
}

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const MemPolicy& value)
{
	json.openStruct();
		switch (value.mode)
		{
			case MPOL_DEFAULT:
				json.printField("mode","MPOL_DEFAULT");
				break;
			case MPOL_BIND:
				json.printField("mode","MPOL_BIND");
				break;
			case MPOL_INTERLEAVE:
				json.printField("mode","MPOL_INTERLEAVE");
				break;
			case MPOL_PREFERRED:
				json.printField("mode","MPOL_PREFERRED");
				break;
			case MPOL_LOCAL:
				json.printField("mode","MPOL_LOCAL");
				break;
			default:
				json.printField("mode",value.mode);
				break;
		}

		json.printField("type",getMemBindTypeName(value.type));
		
		json.openFieldArray("mask");
			for (size_t i = 0 ; i < sizeof (value.mask) * 8 ; i++)
				if (value.mask[i/64] & (1lu << (i%64)))
					json.printValue(i);
		json.closeFieldArray("mask");
	json.closeStruct();
}

}
