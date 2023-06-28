/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef THREAD_TRACKER_HPP
#define THREAD_TRACKER_HPP

/********************  HEADERS  *********************/
#include <map>
#include <list>
#include "../../../extern-deps/from-htopml/json/ConvertToJson.h"
#include "ProcessTracker.hpp"
#include "MallocTracker.hpp"
#include "Stack.hpp"
#include "AccessMatrix.hpp"
#include "../portability/Clock.hpp"
#include "../portability/OS.hpp"
#include "../common/StaticAssoCache.hpp"
#include "../caches/CpuCache.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  STRUCT  *********************/
/**
 * Entrise of the log of thread pinning.
**/
struct ThreadBindingLogEntry
{
	/** At which time the thread as moved. **/
	ClockValue at;
	/** The NUMA node to which it is assigigned (-1 if not binded) . **/
	int numa;
};

/*********************  STRUCT  *********************/
/**
 * Entry the the memory  binding pinning log.
**/
struct ThreadMemBindingLogEntry
{
	/** At which time the memory has been pinned **/
	ClockValue at;
	/** The policy it uses in set_mempolicy **/
	MemPolicy policy;
};

/*********************  STRUCT  *********************/
/**
 * Used to aggregate accesses and handle many of them by batch.
**/
struct AccessEvent
{
	/** Instruction pointer **/
	size_t ip;
	/** Memory address accessed **/
	size_t addr;
	/** Is read of write **/
	bool write;
	/** Do we skip counter **/
	bool skip;
};

/*********************  TYPES  **********************/
/** 
 * Build cache of instruction and alloc counters. 
 * it is indexed by the real counter address so we can dump
 * quicly the cache into the final struct wihtout running
 * over the std::map. So we can use directly atomics.
**/
typedef std::map<Stats *,Stats> AllocCacheMap;
/** List to build the thread binding log **/
typedef std::list<ThreadBindingLogEntry> ThreadBindingLog;
/** Lst to build the thread memory binding log **/
typedef std::list<ThreadMemBindingLogEntry> MemPolicyLog;
/**
 * used to batch accesses handling to avoid to interupt too much the
 * program flow.
**/
typedef std::vector<AccessEvent> AccessVector;
/** Define a TLB cache to avoid running over all the page table every time. **/
typedef StaticAssoCache<Page,4,32> TLB;
/** Define an instruction/alloc cache to find counters quicly **/
typedef StaticAssoCache<Stats,4,64> CounterCache;

/*********************  CLASS  **********************/
/**
 * Object to track the states of a thread.
**/
class ThreadTracker
{
	public:
		ThreadTracker(ProcessTracker * process);
		void flushAllocCache(void);
		void flush(void);
		void onAccess(size_t ip,size_t addr,bool write,bool skip = false);
		void onAccessHandling(size_t ip,size_t addr,bool write,bool skip = false);
		void onSetAffinity(cpu_set_t * mask,int size);
		void onStop(void);
		void onMunmap(size_t addr,size_t size);
		void onMmap(size_t addr,size_t size,size_t flags,size_t fd);
		void onMremap(size_t oldAddr,size_t oldSize,size_t newAddr, size_t newSize);
		void onAlloc(size_t ip,size_t ptr,size_t size);
		void onFree(size_t ptr);
		void onRealloc(size_t ip, size_t oldPtr, size_t newPtr, size_t newSize);
		void onEnterFunction(void * addr);
		void onExitFunction(void);
		void onSetMemPolicy(int mode,const unsigned long * mask,unsigned long maxNodes);
		void onMBind(void * addr,size_t len,size_t mode,const unsigned long *nodemask,size_t maxnode,size_t flags);
		int getTID(void);
		friend void convertToJson(htopml::JsonState& json, const ThreadTracker& value);
	private:
		void logBinding(int numa);
		void logBinding(MemPolicy & policy);
		bool isMemBind(void);
		void flushAccessBatch(void);
	private:
		/** Linux thread ID **/
		int tid;
		/** Pointer to the parrent process tracker **/
		ProcessTracker * process;
		/** Instruction counter cache. **/
		InstrInfoMap instructions;
		/** Allocation cache **/
		AllocCacheMap allocCache;
		/** Globl access counters for the thread. **/
		Stats stats;
		/** NUMA node thread pinning. **/
		int numa;
		/** Pointer to the process page table. **/
		PageTable * table;
		/** TLB cache to avoid running into the page table every time **/
		TLB tlb;
		/** Instruction cache to go faster **/
		CounterCache icache;
		/** Allocation cache to go faster **/
		CounterCache acache;
		/** Allocation tracker. We build one per thread to eliminate locks. **/
		MallocTracker allocTracker;
		/** Counter for dummy allocations (static objects like global variables, consts...). **/
		Stats dummyAlloc;
		/** Pointer to the NUMA topology **/
		NumaTopo * topo;
		/** Track the call stack of the thread. **/
		Stack stack;
		/** Access matrix of the current thread **/
		AccessMatrix accessMatrix;
		/** CPU on which the thread is allowed to run on **/
		CpuBindList cpuBindList;
		/** Mutex to protect access to the binding logs **/
		Mutex bindingLogMutex;
		/** Log to keep track of the thread binding **/
		ThreadBindingLog bindingLog;
		/** Remeber when the thread was born. **/
		ClockValue clockStart;
		/** Remember when the thread died. **/
		ClockValue clockEnd;
		/** Keep track of current memory policy. **/
		MemPolicy memPolicy;
		/** Log the the thread memory policy changes **/
		MemPolicyLog memPolicyLog;
		/** Count number of pages touches for each NUMA node. **/
		size_t * cntTouchedPages;
		/** Number of calls to mbind **/
		size_t mbindCalls;
		/** Count how instruction flush we made to warn the user if there is too many. **/
		size_t instructionFlush;
		/** Count how allocation flush we made to warn the user if there is too many. **/		
		size_t allocFlush;
		/** Number of entries in the instruction cache **/
		size_t cacheEntries;
		/** Counters of access for every NUMA distance. **/
		size_t * distanceCnt;
		/** aggregate accesses to treat them as batch **/
		AccessVector accessBatch;
		/** Store the local CPU cache simulator handler **/
		CpuCache * cpuCache;
		/** To gain one function call if cache simulator is set to dummy or not **/
		bool hasCpuCache;
};

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const ThreadBindingLogEntry& value);
void convertToJson(htopml::JsonState& json, const ThreadMemBindingLogEntry& value);
void convertToJson(htopml::JsonState& json, const MemPolicy& value);

}

#endif //THREAD_TRACKER_HPP
