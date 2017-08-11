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
#include "../common/Helper.hpp"
#include "../portability/OS.hpp"
#include "../portability/Clock.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <fstream>

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
ProcessTracker::ProcessTracker(void)
{
	//setup page counters
	int nodes = topo.getNumaNodes();
	currentAllocatedPages.reserve(nodes);
	maxAllocatedPages.reserve(nodes);
	for (int i = 0 ; i < nodes ; i++)
	{
		currentAllocatedPages.push_back(0);
		maxAllocatedPages.push_back(0);
	}

	//to be sure clock is init
	Clock::get();
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
int ProcessTracker::getNumaAffinity(CpuBindList * cpuBindList)
{
	return topo.getCurrentNumaAffinity(cpuBindList);
}

/*******************  FUNCTION  *********************/
int ProcessTracker::getNumaAffinity(cpu_set_t * mask, int size,CpuBindList * cpuBindList)
{
	return topo.getCurrentNumaAffinity(mask,size,cpuBindList);
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
void ProcessTracker::onAfterFirstTouch(int pageNuma)
{
	size_t res = __sync_add_and_fetch(&currentAllocatedPages[pageNuma],1,__ATOMIC_RELAXED);
	if (res > maxAllocatedPages[pageNuma])
		__atomic_store(&maxAllocatedPages[pageNuma],&res,__ATOMIC_RELAXED);
}

/*******************  FUNCTION  *********************/
void ProcessTracker::onMunmap(size_t baseAddr,size_t size)
{
	//seutp
	uint64_t end = (uint64_t)baseAddr + size;
	uint64_t start = ((uint64_t)baseAddr) & (~NUMAPROF_PAGE_MASK);
	
	//loop
	for (uint64_t addr = start; addr < end ; addr += NUMAPROF_PAGE_SIZE)
	{
		Page & page = pageTable.getPage(addr);
		if (page.numaNode >= 0)
			__atomic_sub_fetch(&currentAllocatedPages[page.numaNode],1,__ATOMIC_RELAXED);
	}

	//clear page table
	pageTable.clear(baseAddr,size);
}

/*******************  FUNCTION  *********************/
void ProcessTracker::onThreadSetAffinity(int pid,cpu_set_t * mask, int size)
{
	bool found = false;
	mutex.lock();
	for (ThreadTrackerMap::iterator it = threads.begin() ; it != threads.end() ; ++it)
		if (it->second->getTID() == pid)
		{
			found = true;
			it->second->onSetAffinity(mask,size);
			break;
		}
	mutex.unlock();

	//error
	if (found == false)
		printf("WARNING, failed to found TID %d for binding, is it from an external process ?\n",pid);
}

/*******************  FUNCTION  *********************/
void ProcessTracker::onExit(void)
{
	//get clock
	clockAtEnd = Clock::get();

	//flush local data
	for (ThreadTrackerMap::iterator it = threads.begin() ; it != threads.end() ; ++it)
		it->second->flush();
	
	//extract symbols
	registry.loadProcMap();

	#ifdef NUMAPROG_CALLSTACK
		for (InstrInfoMap::iterator it = instructions.begin() ; it != instructions.end() ; ++it)
			for (int i = 0 ; i < NUMAPROG_MINI_STACk_SIZE ; i++)
				if (it->first.stack[i] != NULL)
					registry.registerAddress((void*)(it->first.stack[i]));
		for (InstrInfoMap::iterator it = allocStats.begin() ; it != allocStats.end() ; ++it)
			for (int i = 0 ; i < NUMAPROG_MINI_STACk_SIZE ; i++)
				if (it->first.stack[i] != NULL)
					registry.registerAddress((void*)(it->first.stack[i]));
	#else
		for (InstrInfoMap::iterator it = instructions.begin() ; it != instructions.end() ; ++it)
			registry.registerAddress((void*)(it->first));
		for (InstrInfoMap::iterator it = allocStats.begin() ; it != allocStats.end() ; ++it)
			registry.registerAddress((void*)(it->first));
	#endif
	registry.solveNames();

	//prep filename
	char buffer[64];
	sprintf(buffer,"numaprof-%d.json",OS::getPID());
	
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
		json.openFieldStruct("infos");
			json.printField("formatVersion",1);
			json.printField("tool","numaprof");
			json.printField("exe",OS::getExeName());
			json.printField("command",OS::getCmdLine());
			json.printField("hostname",OS::getHostname());
			json.printField("date",OS::getDateTime());
			json.printField("duration",value.clockAtEnd);
		json.closeFieldStruct("infos");
		json.openFieldStruct("process");
			json.printField("maxAllocatedPages",value.maxAllocatedPages);
		json.closeFieldStruct("process");
		json.printField("threads",value.threads);
		json.printField("topo",value.topo);
		json.printField("symbols",value.registry);
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
