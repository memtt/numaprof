/*****************************************************
             PROJECT  : numaprof
             VERSION  : 0.0.0-dev
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include "MallocTracker.hpp"
#include "ProcessTracker.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
/**
 * Constructor og the tracker.
 * @param pageTable Pointer to the page table to register allocation regerences.
**/
MallocTracker::MallocTracker(PageTable * pageTable)
{
	this->pageTable = pageTable;
}

/*******************  FUNCTION  *********************/
/**
 * To be called by thread to register the allocation into the page table
 * and point the instruction site corresponding the malloc site.
 * Remark, we store an instruction map on every thread, this increase
 * the memory usage but avoid locking. The instructino map are
 * merged into the process tracker at the end of the execution.
 * @param ip Instruction of call stack defining the malloc call site.
 * @param ptr Address of the allocated segement
 * @param size size of the allocated segement.
**/
void MallocTracker::onAlloc(StackIp & ip,size_t ptr, size_t size)
{
	//printf("On alloc : (%p) %p => %lu\n",(void*)ip,(void*)ptr,size);
	//get mini stack

	//allocate info
	MallocInfos * infos = new MallocInfos;
	infos->ptr = ptr;
	infos->size = size;
	infos->stats = &(instructions[ip]);

	//debug
	#ifdef NUMAPROF_TRACE_ALLOCS
		printf("TRACE: tracker.onAlloc(ip,%p,%lu);\n",(void*)ptr,size);
	#endif
	
	//reg to page table
	pageTable->regAllocPointer(ptr,size,infos);
}

/*******************  FUNCTION  *********************/
/**
 * To be called when allocated segments are freed by the application.
 * It unregister the segement from the page table and free the temporary
 * structure.
 * @param ptr Pointer of the allocation.
**/
void MallocTracker::onFree(size_t ptr)
{
	//trivial
	if (ptr == 0)
		return;

	//get infos
	Page & page = pageTable->getPage(ptr);
	MallocInfos * infos = (MallocInfos *)page.getAllocPointer(ptr);

	//not ok
	if (infos == NULL || infos->ptr != ptr)
		return;

	//debug
	#ifdef NUMAPROF_TRACE_ALLOCS
		printf("TRACE: tracker.onFree(%p);\n",(void*)ptr);
	#endif
	
	//free into page table
	pageTable->freeAllocPointer(ptr,infos->size,infos);

	//free mem
	delete infos;
}

/*******************  FUNCTION  *********************/
/**
 * At the end of the program execution dump the per thread
 * counters into the global process one.
**/
void MallocTracker::flush(class ProcessTracker * process)
{
	process->mergeAllocInstruction(instructions);
}

}
