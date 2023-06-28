/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef PROCESS_TRACKER_HPP
#define PROCESS_TRACKER_HPP

/********************  HEADERS  *********************/
#include <map>
#include "PageTable.hpp"
#include "../portability/Mutex.hpp"
#include "../portability/NumaTopo.hpp"
#include "../portability/Clock.hpp"
#include "Stats.hpp"
#include "../../../extern-deps/from-htopml/json/ConvertToJson.h"
#include "../../../extern-deps/from-malt-v2/SymbolRegistry.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  TYPES  **********************/
//to break loop dependencies
class ThreadTracker;
/** Keep track of all thred handlers **/
typedef std::map<int,ThreadTracker *> ThreadTrackerMap;
/** To track counter for each NUMA node **/
typedef std::vector<size_t> AllocatedPageCounter;
/** 
 * Track loaded binary so files to mark new onces as not pinned if
 * the related option is enabled.
**/
typedef std::map<std::string,bool> LoadedObjectMap;

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const ThreadTrackerMap& value);

/*********************  CLASS  **********************/
/**
 * Track the status of the current process and aggregate all the data of threads
 * to dump them in the final profile file.
**/
class ProcessTracker
{
	public:
		ProcessTracker(void);
		ThreadTracker * createThreadTracker(int threadId);
		void mergeInstruction(InstrInfoMap & stats);
		void mergeAllocInstruction(InstrInfoMap & stats);
		int getNumaAffinity(CpuBindList * cpuBindList);
		int getNumaAffinity(cpu_set_t * mask, int size, CpuBindList * cpuBindList);
		PageTable * getPageTable(void);
		friend void convertToJson(htopml::JsonState& json, const ProcessTracker& value);
		void onExit(void);
		void onAfterFirstTouch(int pageNuma,size_t pages);
		void onMunmap(size_t baseAddr,size_t size);
		NumaTopo & getNumaTopo(void);
		void onThreadSetAffinity(int pid,cpu_set_t * mask,int size);
		void markObjectFiledAsNotPinned(void);
		void * getCpuCacheLayout(void);
		void registerLibBaseAddr(const std::string & lib, size_t baseAddr);
	private:
		void removeSmall(InstrInfoMap & map, float cutoff);
		size_t getProfileId(void) const;
	private:
		/**
		 * Keep track of the process page table to avoid calling move_pages
		 * every time. It also permit to find the allocated chunk counters
		 * when accessing a dynamically allocated address.
		**/
		PageTable pageTable;
		/** Global container to store counters for each instruction of the instrumented program. **/
		InstrInfoMap instructions;
		/** Global conatiner to store the counters for each allocation site. **/
		InstrInfoMap allocStats;
		/** Keep track of the thread trackers. **/
		ThreadTrackerMap threads;
		/** Mutex to protect the object on parallel accesses **/
		Mutex mutex;
		/** Load the NUMA topology of the current machine we run on. **/
		NumaTopo topo;
		/** Symbol registry to resove function addresses. **/
		MATT::SymbolRegistry registry;
		/** Keep track of pages allocated on each NUMA node. **/
		AllocatedPageCounter currentAllocatedPages;
		/** Keep track of the max pages allocated on each NUMA node.**/
		AllocatedPageCounter maxAllocatedPages;
		/** To build the time percentage of pinning events **/
		ClockValue clockAtEnd;
		/** List of loaded object (binary & so files) to detect new ones **/
		LoadedObjectMap loadedObjects;
		/** Store the global cache layout **/
		void * cpuCacheLayout;
};

}

#endif //PROCESS_TRACKER_HPP
