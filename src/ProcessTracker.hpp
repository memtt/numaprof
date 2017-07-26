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
#include "Mutex.hpp"
#include "NumaTopo.hpp"
#include "Stats.hpp"
#include "../extern-deps/from-htopml/json/ConvertToJson.h"
#include "../extern-deps/from-malt-v2/SymbolRegistry.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  TYPES  **********************/
class ThreadTracker;
typedef std::map<int,ThreadTracker *> ThreadTrackerMap;

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
		int getNumaAffinity(void);
		int getNumaAffinity(cpu_set_t * mask);
		PageTable * getPageTable(void);
		friend void convertToJson(htopml::JsonState& json, const ProcessTracker& value);
		void onExit(void);
		NumaTopo & getNumaTopo(void);
	private:
		PageTable pageTable;
		InstrInfoMap instructions;
		InstrInfoMap allocStats;
		ThreadTrackerMap threads;
		Mutex mutex;
		NumaTopo topo;
		MATT::SymbolRegistry registry;
};

}

#endif //PROCESS_TRACKER_HPP
