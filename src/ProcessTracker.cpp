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
void ProcessTracker::mergeAllocInstruction(InstrInfoMap & stats)
{
	mutex.lock();
	for (InstrInfoMap::iterator it = stats.begin() ; it != stats.end() ; ++it)
		allocStats[it->first].merge(it->second);
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
NumaTopo & ProcessTracker::getNumaTopo(void)
{
	return topo;
}

/*******************  FUNCTION  *********************/
PageTable * ProcessTracker::getPageTable(void)
{
	return &pageTable;
}

/*******************  FUNCTION  *********************/
void ProcessTracker::onExit(void)
{
	//flush local data
	for (ThreadTrackerMap::iterator it = threads.begin() ; it != threads.end() ; ++it)
		it->second->flush();

	//prep filename
	char buffer[64];
	sprintf(buffer,"numaprof-%d.json",getpid());

	//open & dump
	std::ofstream out;
	out.open(buffer);
	htopml::convertToJson(out,*this,true);
	out.close();
}

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const ProcessTracker& value)
{
	json.openStruct();
		json.printField("threads",value.threads);
		json.printField("instructions",value.instructions);
		json.printField("allocs",value.allocStats);
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

}
