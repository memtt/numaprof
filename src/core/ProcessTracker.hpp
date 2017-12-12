/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
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
#include "../../extern-deps/from-htopml/json/ConvertToJson.h"
#include "../../extern-deps/from-malt-v2/SymbolRegistry.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  TYPES  **********************/
class ThreadTracker;
typedef std::map<int,ThreadTracker *> ThreadTrackerMap;
typedef std::vector<size_t> AllocatedPageCounter;

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const ThreadTrackerMap& value);

/*********************  CLASS  **********************/
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
	private:
		void removeSmall(InstrInfoMap & map, float cutoff);
	private:
		PageTable pageTable;
		InstrInfoMap instructions;
		InstrInfoMap allocStats;
		ThreadTrackerMap threads;
		Mutex mutex;
		NumaTopo topo;
		MATT::SymbolRegistry registry;
		AllocatedPageCounter currentAllocatedPages;
		AllocatedPageCounter maxAllocatedPages;
		ClockValue clockAtEnd;
};

}

#endif //PROCESS_TRACKER_HPP
