/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef THREAD_TRACKER_HPP
#define THREAD_TRACKER_HPP

/********************  HEADERS  *********************/
#include <map>
#include <list>
#include "../../extern-deps/from-htopml/json/ConvertToJson.h"
#include "ProcessTracker.hpp"
#include "MallocTracker.hpp"
#include "Stack.hpp"
#include "AccessMatrix.hpp"
#include "../portability/Clock.hpp"
#include "../portability/OS.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  STRUCT  *********************/
struct ThreadBindingLogEntry
{
	ClockValue at;
	int numa;
};

/*********************  STRUCT  *********************/
struct ThreadMemBindingLogEntry
{
	ClockValue at;
	MemPolicy policy;
};

/*********************  TYPES  **********************/
typedef std::map<Stats *,Stats> AllocCacheMap;
typedef std::list<ThreadBindingLogEntry> ThreadBindingLog;
typedef std::list<ThreadMemBindingLogEntry> MemPolicyLog;

/*********************  CLASS  **********************/
class ThreadTracker
{
	public:
		ThreadTracker(ProcessTracker * process);
		void flush(void);
		void onAccess(size_t ip,size_t addr,bool write);
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
	private:
		int tid;
		ProcessTracker * process;
		InstrInfoMap instructions;
		AllocCacheMap allocCache;
		Stats stats;
		int numa;
		PageTable * table;
		MallocTracker allocTracker;
		Stats dummyAlloc;
		NumaTopo * topo;
		Stack stack;
		AccessMatrix accessMatrix;
		CpuBindList cpuBindList;
		Mutex bindingLogMutex;
		ThreadBindingLog bindingLog;
		ClockValue clockStart;
		ClockValue clockEnd;
		MemPolicy memPolicy;
		MemPolicyLog memPolicyLog;
		size_t * cntTouchedPages;
		size_t mbindCalls;
};

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const ThreadBindingLogEntry& value);
void convertToJson(htopml::JsonState& json, const ThreadMemBindingLogEntry& value);
void convertToJson(htopml::JsonState& json, const MemPolicy& value);

}

#endif //THREAD_TRACKER_HPP
