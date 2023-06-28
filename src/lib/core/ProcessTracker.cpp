/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include "ProcessTracker.hpp"
#include "ThreadTracker.hpp"
#include "../common/Debug.hpp"
#include "../common/Helper.hpp"
#include "../portability/OS.hpp"
#include "../portability/Clock.hpp"
#include "../common/Options.hpp"
#include "../common/FormattedMessage.hpp"
#include "../caches/CpuCacheBuilder.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <fstream>

/********************  MACRO  ***********************/
#define NUMAPROF_OTHERS (0x2LU)

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
/**
 * Constructor of the process tracker, mostly setup all the internal states, loading
 * the NUMA topology and allocates per NUMA node variables.
**/
ProcessTracker::ProcessTracker(void)
{
	//setup page counters
	int nodes = topo.getNumaNodes();
	currentAllocatedPages.reserve(nodes);
	maxAllocatedPages.reserve(nodes);
	markObjectFiledAsNotPinned();
	for (int i = 0 ; i < nodes ; i++)
	{
		currentAllocatedPages.push_back(0);
		maxAllocatedPages.push_back(0);
	}

	//build cpu cache simulator
	cpuCacheLayout = CpuCacheBuilder::buildLayout(getGlobalOptions().cacheType);

	//to be sure clock is init
	Clock::get();
}

/*******************  FUNCTION  *********************/
/**
 * Allocate a thread tracker and register if to the process.
 * @param threadId Define the thread ID provided by pintool. If ID is recycled the
 * function return the old thread tracker to resuse it.
**/
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
	
	//debug
	if (getGlobalOptions().outputSilent == false)
		fprintf(stderr,"NUMAPROF: register thread %d\n",ret->getTID());
	
	//unlock
	mutex.unlock();

	//ret
	return ret;
}

/*******************  FUNCTION  *********************/
/**
 * Use by thread tracker to merge its instruction counter cache
 * when it becomes full. Caution it take locks so should limit
 * usage.
 * @param stats Cache to be merged into the global conatainer.
**/
void ProcessTracker::mergeInstruction(InstrInfoMap & stats)
{
	mutex.lock();
	for (InstrInfoMap::iterator it = stats.begin() ; it != stats.end() ; ++it)
		instructions[it->first].merge(it->second);
	mutex.unlock();
}

/*******************  FUNCTION  *********************/
/**
 * Use by thread tracker to merge its allocation sites counter cache
 * when it becomes full. Caution it take locks so should limit
 * usage.
 * @param stats Cache to be merged into the global conatainer.
**/
void ProcessTracker::mergeAllocInstruction(InstrInfoMap & stats)
{
	mutex.lock();
	for (InstrInfoMap::iterator it = stats.begin() ; it != stats.end() ; ++it)
		allocStats[it->first].merge(it->second);
	mutex.unlock();
}

/*******************  FUNCTION  *********************/
/**
 * Use the topology to summarize the NUMA mapping from the given 
 * mask. If the thread is able to run on multiple NUMA node the function
 * return -1.
 * @param cpuBindList List of CPUs on which the thread can run on.
**/
int ProcessTracker::getNumaAffinity(CpuBindList * cpuBindList)
{
	return topo.getCurrentNumaAffinity(cpuBindList);
}

/*******************  FUNCTION  *********************/
/**
 * Use the topology to summarize the NUMA mapping from the given 
 * mask. If the thread is able to run on multiple NUMA node the function
 * return -1.
 * @param mask Define the mask parameter from sched_setaffinity interception
 * @param size Define size of mask (also comes from sched_setaffinity).
 * @param cpuBindList List of CPUs on which the thread can run on.
**/
int ProcessTracker::getNumaAffinity(cpu_set_t * mask, int size,CpuBindList * cpuBindList)
{
	return topo.getCurrentNumaAffinity(mask,size,cpuBindList);
}

/*******************  FUNCTION  *********************/
/**
 * Return a reference to the NUMA topology container.
**/
NumaTopo & ProcessTracker::getNumaTopo(void)
{
	return topo;
}

/*******************  FUNCTION  *********************/
/**
 * Return a reference to the page table.
**/
PageTable * ProcessTracker::getPageTable(void)
{
	return &pageTable;
}

/*******************  FUNCTION  *********************/
/**
 * To be called by thread handler on first touch to update the number of pages
 * allocated on each NUMA node.
**/
void ProcessTracker::onAfterFirstTouch(int pageNuma,size_t pages)
{
	size_t res = __sync_add_and_fetch(&currentAllocatedPages[pageNuma],pages,__ATOMIC_RELAXED);
	if (res > maxAllocatedPages[pageNuma])
		__atomic_store(&maxAllocatedPages[pageNuma],&res,__ATOMIC_RELAXED);
}

/*******************  FUNCTION  *********************/
/**
 * To be called when munma is called by the process to update the page table
 * and mark all the pages as freed.
 * @param baseAddr Base address of the segment.
 * @param size Size of the segment.
**/
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
/**
 * Function to be called when using sched_setaffinity to bind another thread in the process.
 * @param pid PID of the thread to move.
 * @param mask New mapping of the thread.
 * @param size Size of the mask.
**/
void ProcessTracker::onThreadSetAffinity(int pid,cpu_set_t * mask, int size)
{
	bool found = false;
	int retry = 0;
	
	//ok sometime if seams to be called before the other thread is registered
	//so we make several tries before really failing and ignore
	while(found == false && retry < 200)
	{
		mutex.lock();
		for (ThreadTrackerMap::iterator it = threads.begin() ; it != threads.end() ; ++it)
			if (it->second->getTID() == pid)
			{
				found = true;
				it->second->onSetAffinity(mask,size);
				break;
			}
		mutex.unlock();
		
		//not found to retry
		if (found == false)
			usleep(500);
		retry++;
	}
	
	//info
	if (!getGlobalOptions().outputSilent && retry > 1)
		fprintf(stderr,"NUMAPROF: had to retry (%d) to find thread\n",retry);

	//error
	if (found == false)
		fprintf(stderr,"NUMAPROF WARNING, failed to found TID %d for binding, is it from an external process ?\n",pid);
}

/*******************  FUNCTION  *********************/
/**
 * Return the PID of the RANK depending on the config.
**/
size_t ProcessTracker::getProfileId(void) const
{
	//get options
	const Options & options = getGlobalOptions();

	//switch RANK or PID
	if (options.mpiUseRank == false) {
		//PID is trivial
		return OS::getPID();
	} else if (options.mpiRankVar == "auto") {
		//we need to search for env vars
		if (getenv("OMPI_COMM_WORLD_RANK") != NULL)
			return atol(getenv("OMPI_COMM_WORLD_RANK"));
		else if (getenv("PMI_RANK") != NULL)
			return atol(getenv("PMI_RANK"));
		else
			numaprofFatal("Fail to determine the env variable to be used to extract the MPI rank. Please define it via -o mpi:rankVar.");
	} else if (Helper::isInteger(options.mpiRankVar)) {
		//provide directly the ID
		return atol(options.mpiRankVar.c_str());
	} else {
		//error
		numaprofFatal("Get invalid value for mpi:rankVar !");
	}

	//default in case & to avoid warning
	return OS::getPID();
}

/*******************  FUNCTION  *********************/
/**
 * To be called when the process exit to dump the final profile file.
**/
void ProcessTracker::onExit(void)
{
	//get clock
	clockAtEnd = Clock::get();
	
	//dump config
	const Options & options = getGlobalOptions();
	if (options.outputDumpConfig)
		options.dumpConfig(FormattedMessage(options.outputName).arg(OS::getExeName()).arg(this->getProfileId()).arg("ini").toString().c_str());
	
	//flush alloc cache data
	//this must be done before calling flush because otherwise
	//we miss some data as this cache point on struct from every threads
	for (ThreadTrackerMap::iterator it = threads.begin() ; it != threads.end() ; ++it)
		it->second->flushAllocCache();

	//flush local data
	for (ThreadTrackerMap::iterator it = threads.begin() ; it != threads.end() ; ++it)
		it->second->flush();
	
	//remove small
	if (getGlobalOptions().outputRemoveSmall)
	{
		printf("NUMAPROF Do remove values bellow : %f%%\n",getGlobalOptions().outputRemoveRatio);
		this->removeSmall(instructions,getGlobalOptions().outputRemoveRatio);
		this->removeSmall(allocStats,getGlobalOptions().outputRemoveRatio);
	}
	
	//extract symbols
	registry.loadProcMap();

	#ifdef NUMAPROF_CALLSTACK
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

	//dump json file
	if (getGlobalOptions().outputJson)
	{
		//open & dump
		std::string fname = FormattedMessage(options.outputName).arg(OS::getExeName()).arg(this->getProfileId()).arg("json").toString();
		std::ofstream out;
		fprintf(stderr,"NUMAPROF: Dump profile in %s\n",fname.c_str());
		out.open(fname.c_str());
		htopml::convertToJson(out,*this,getGlobalOptions().outputIndent);
		out.close();
	}
}

/*******************  FUNCTION  *********************/
/**
 * Before dumping the final profile, appli a cut-off on the counters.
 * If the value of the counter is too small, the instruction if removed from the profile
 * and its counters merge on the global "other" counters.
 * @param map Instruction map too loop on.
 * @param cutoff Value to use to cutoff (in percent).
**/
void ProcessTracker::removeSmall(InstrInfoMap & map,float cutoff)
{
	//vars
	Stats global;
	Stats others;
	InstrInfoMap copy;
	
	//compute global by merging & build a copy (Ok, I should use C++11 move operator instead)
	for (InstrInfoMap::iterator it = map.begin() ; it != map.end() ; ++it)
	{
		global.merge(it->second);
		copy[it->first] = it->second;
	}
	
	//clear origin
	map.clear();
	
	//copute cut
	Stats cut;
	cut.merge(global);
	cut.applyCut(cutoff);
	
	//copy only the one larger than 0.1%
	for (InstrInfoMap::iterator it = copy.begin() ; it != copy.end() ; ++it)
	{
		//check if one is larger than cutoff
		if (it->second.asOneLargerThan(cut))
			map[it->first] = it->second;
		else
			others.merge(it->second);
	}
	
	//insert others
	map[NUMAPROF_OTHERS] = others;
}

/*******************  FUNCTION  *********************/
/**
 * Mark all the SO/binary ifles as unpinned.
**/
void ProcessTracker::markObjectFiledAsNotPinned(void)
{
	//check
	if (gblOptions->coreObjectCodePinned == true)
		return;
	
	//load page map
	MATT::LinuxProcMapReader reader;
	reader.load();

	//mark new as not pinned
	for (MATT::LinuxProcMapReader::const_iterator it = reader.begin() ; it != reader.end() ; ++it)
		if (it->file.empty() == false)
			if (it->file[0] != '[')
				if (loadedObjects.find(it->file) == loadedObjects.end())
					pageTable.markObjectFileAsNotPinned(it->lower,(size_t)it->upper - (size_t)it->lower);

	//remember to not mark again
	for (MATT::LinuxProcMapReader::const_iterator it = reader.begin() ; it != reader.end() ; ++it)
		if (it->file.empty() == false)
			if (it->file[0] != '[')
				loadedObjects[it->file] = true;
}

/*******************  FUNCTION  *********************/
void ProcessTracker::registerLibBaseAddr(const std::string & lib, size_t baseAddr)
{
	this->mutex.lock();
	this->registry.registerLibBaseAddr(lib, baseAddr);
	this->mutex.unlock();
}

/*******************  FUNCTION  *********************/
/**
 * Return the current CPU cache layout to build local thread CPU cache.
**/
void * ProcessTracker::getCpuCacheLayout(void)
{
	return cpuCacheLayout;
}

/*******************  FUNCTION  *********************/
/**
 * Convert the process tracker states into JSON output.
 * @param json JSON state tracker.
 * @param value Process tracker to convert.
**/
void convertToJson(htopml::JsonState& json, const ProcessTracker& value)
{
	json.openStruct();
		json.openFieldStruct("infos");
			json.printField("formatVersion",2);
			json.printField("tool","numaprof");
			if (getGlobalOptions().infoHidden == false)
			{
				json.printField("exe",OS::getExeName());
				json.printField("command",OS::getCmdLine());
				json.printField("hostname",OS::getHostname());
			} else {
				json.printField("exe","unknown");
				json.printField("command","unknown");
				json.printField("hostname","unknown");
			}
			json.printField("date",OS::getDateTime());
			json.printField("duration",value.clockAtEnd);
		json.closeFieldStruct("infos");
		json.printField("config",getGlobalOptions());
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
/**
 * Convert the thread map (all the thread handlers) into JSON format.
 * @param json Json state to generate the output.
 * @param value Degine the thread tracker.
**/
void convertToJson(htopml::JsonState& json, const ThreadTrackerMap& value)
{
	json.openArray();
		for (ThreadTrackerMap::const_iterator it = value.begin() ; it != value.end() ; ++it)
			json.printValue(*(it->second));
	json.closeArray();
}

}
