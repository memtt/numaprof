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

}
