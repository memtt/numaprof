/*****************************************************
			 PROJECT  : numaprof
			 VERSION  : 2.3.0
			 DATE     : 05/2017
			 AUTHOR   : Valat Sébastien - CERN
			 LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include "ProcessTracker.hpp"
#include "ThreadTracker.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <fstream>

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
ProcessTracker::ProcessTracker(void)
{

}

/*******************  FUNCTION  *********************/
ThreadTracker * ProcessTracker::createThreadTracker(int threadId)
{
	//lock
	mutex.lock();

	//check if exist
	ThreadTracker * ret = NULL;
	ThreadTrackerMap::iterator it = threads.find(threadId);
	if (it == threads.end())
		threads[threadId] = ret = new ThreadTracker(this);
	else
		ret = it->second;
	
	//unlock
	mutex.unlock();

	//ret
	return ret;
}

/*******************  FUNCTION  *********************/
void ProcessTracker::mergeInstruction(InstrInfoMap & stats)
{
	mutex.lock();
	for (InstrInfoMap::iterator it = stats.begin() ; it != stats.end() ; ++it)
		instructions[it->first].merge(it->second);
	mutex.unlock();
}

/*******************  FUNCTION  *********************/
int ProcessTracker::getNumaAffinity(void)
{
	return topo.getCurrentNumaAffinity();
}

/*******************  FUNCTION  *********************/
int ProcessTracker::getNumaAffinity(cpu_set_t * mask)
{
	return topo.getCurrentNumaAffinity(mask);
}

/*******************  FUNCTION  *********************/
PageTable * ProcessTracker::getPageTable(void)
{
	return &pageTable;
}

/*******************  FUNCTION  *********************/
void ProcessTracker::onExit(void)
{
	char buffer[64];
	sprintf(buffer,"numaprof-%d.json",getpid());

	std::ofstream out;
	out.open(buffer);
	htopml::convertToJson(out,*this,true);
	out.close();
}

/*******************  FUNCTION  *********************/
InstrInfo::InstrInfo(void)
{
	firstTouch = 0;
	unpinnedFirstTouch = 0;
	unpinnedPageAccess = 0;
	unpinnedThreadAccess = 0;
	unpinnedBothAccess = 0;
	localAccess = 0;
	remoteAccess = 0;
}

/*******************  FUNCTION  *********************/
void InstrInfo::merge(InstrInfo & info)
{
	this->firstTouch += info.firstTouch;
	this->unpinnedFirstTouch += info.unpinnedFirstTouch;
	this->unpinnedPageAccess += info.unpinnedPageAccess;
	this->unpinnedThreadAccess += info.unpinnedThreadAccess;
	this->unpinnedBothAccess += info.unpinnedBothAccess;
	this->localAccess += info.localAccess;
	this->remoteAccess += info.remoteAccess;
}

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const ProcessTracker& value)
{
	json.openStruct();
		json.printField("threads",value.threads);
		json.printField("instructions",value.instructions);
	json.closeStruct();
}

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const InstrInfoMap& value)
{
	json.openStruct();
		for (InstrInfoMap::const_iterator it = value.begin() ; it != value.end() ; ++it)
		{
			char buffer[32];
			sprintf(buffer,"0x%lx",it->first);
				json.printField(buffer,it->second);
		}
	json.closeStruct();
}

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const ThreadTrackerMap& value)
{
	json.openArray();
		for (ThreadTrackerMap::const_iterator it = value.begin() ; it != value.end() ; ++it)
			json.printValue(*(it->second));
	json.closeArray();
}

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const InstrInfo& value)
{
	json.openStruct();
		if (value.firstTouch > 0)
			json.printField("firstTouch",value.firstTouch);
		if (value.unpinnedFirstTouch > 0)
			json.printField("unpinnedFirstTouch",value.unpinnedFirstTouch);
		if (value.unpinnedPageAccess > 0)
			json.printField("unpinnedPageAccess",value.unpinnedPageAccess);
		if (value.unpinnedThreadAccess > 0)
			json.printField("unpinnedThreadAccess",value.unpinnedThreadAccess);
		if (value.unpinnedBothAccess > 0)
			json.printField("unpinnedBothAccess",value.unpinnedBothAccess);
		if (value.localAccess > 0)
			json.printField("localAccess",value.localAccess);
		if (value.remoteAccess > 0)
			json.printField("remoteAccess",value.remoteAccess);
	json.closeStruct();
}

}
