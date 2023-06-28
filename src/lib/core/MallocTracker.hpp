/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include "PageTable.hpp"
#include "Stats.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/********************  MACROS  **********************/
//#define NUMAPROF_TRACE_ALLOCS	

/*******************  STRUCT  ***********************/
/**
 * Malloc informations to store for every allocation.
**/
struct MallocInfos
{
	/** base pointer of the allocation. **/
	size_t ptr;
	/** Size of the allocation to know wich range to cleanup on removal. **/
	size_t size;
	/** Pointer to the stat counters (the one related to the malloc call site) **/
	Stats * stats;
};

/*********************  TYPES  **********************/
/** 
 * Define the index of the std::map storing the data.
 * Depend on availability of stack information of just call site.
**/
#ifdef NUMAPROF_CALLSTACK
	typedef MiniStack StackIp;
#else
	typedef size_t StackIp;
#endif

/*******************  FUNCTION  *********************/
/**
 * Object use to track the allocation statistics. Should create one instance per thread.
**/
class MallocTracker
{
	public:
		MallocTracker(PageTable * pageTable);
		void onAlloc(StackIp & ip,size_t ptr, size_t size);
		void onFree(size_t ptr);
		void flush(class ProcessTracker * process);
	private:
		/** Pointer to the page table to know where to register the allocations **/
		PageTable * pageTable;
		/** Instruction map to store the per malloc call site counters **/
		InstrInfoMap instructions;
};

}
