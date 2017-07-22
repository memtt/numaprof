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
#include "../extern-deps/from-htopml/json/ConvertToJson.h"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  STRUCT  *********************/
struct InstrInfo
{
	//functions
	InstrInfo(void);
	void merge(InstrInfo & info);

	//members
	size_t firstTouch;
	size_t unpinnedFirstTouch;
	size_t unpinnedPageAccess;
	size_t unpinnedThreadAccess;
	size_t unpinnedBothAccess;
	size_t localAccess;
	size_t remoteAccess;
};

/*********************  TYPES  **********************/
typedef std::map<size_t,InstrInfo> InstrInfoMap;
class ThreadTracker;
typedef std::map<int,ThreadTracker *> ThreadTrackerMap;

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const InstrInfo& value);
void convertToJson(htopml::JsonState& json, const InstrInfoMap& value);
void convertToJson(htopml::JsonState& json, const ThreadTrackerMap& value);

/*********************  CLASS  **********************/
class ProcessTracker
{
	public:
		ProcessTracker(void);
		ThreadTracker * createThreadTracker(int threadId);
		void mergeInstruction(InstrInfoMap & stats);
		int getNumaAffinity(void);
		int getNumaAffinity(cpu_set_t * mask);
		PageTable * getPageTable(void);
		friend void convertToJson(htopml::JsonState& json, const ProcessTracker& value);
		void onExit(void);
	private:
		PageTable pageTable;
		InstrInfoMap instructions;
		ThreadTrackerMap threads;
		Mutex mutex;
		NumaTopo topo;
};

}

#endif //PROCESS_TRACKER_HPP
