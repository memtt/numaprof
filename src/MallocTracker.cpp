/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
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
MallocTracker::MallocTracker(PageTable * pageTable)
{
	this->pageTable = pageTable;
}

/*******************  FUNCTION  *********************/
void MallocTracker::onAlloc(size_t ip,size_t ptr, size_t size)
{
	//printf("On alloc : (%p) %p => %lu\n",(void*)ip,(void*)ptr,size);

	//allocate info
	MallocInfos * infos = new MallocInfos;
	infos->ptr = ptr;
	infos->size = size;
	infos->stats = &(instructions[ip]);

	//reg to page table
	pageTable->regAllocPointer(ptr,size,infos);
}

/*******************  FUNCTION  *********************/
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

	//free into page table
	pageTable->freeAllocPointer(ptr,infos->size,infos);

	//free mem
	delete infos;
}

/*******************  FUNCTION  *********************/
void MallocTracker::flush(class ProcessTracker * process)
{
	process->mergeAllocInstruction(instructions);
}

}
