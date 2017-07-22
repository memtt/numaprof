/*****************************************************
			 PROJECT  : numaprof
			 VERSION  : 2.3.0
			 DATE     : 05/2017
			 AUTHOR   : Valat Sébastien - CERN
			 LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <cstdint>
#include <iostream>
#include "PageTable.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
Page & PageTable::getPage(void * addr)
{
	//convert
	uint64_t ptr = (uint64_t)addr;

	//get offsets
	int pteOffset = NUMAPROF_PAGE_LEVEL_ID(ptr,0);
	int pmdOffset = NUMAPROF_PAGE_LEVEL_ID(ptr,1);
	int pudOffset = NUMAPROF_PAGE_LEVEL_ID(ptr,2);
	int pgdOffset = NUMAPROF_PAGE_LEVEL_ID(ptr,3);
	
	//dbeug
	//std::cout << pgdOffset << " " << pudOffset << " " << pmdOffset << " " << pteOffset << std::endl;
	
	//get pud
	PageUpperDirectory * pud = pgd.getEntry(pgdOffset);
	if (pud == NULL)
		pud = pgd.makeNewEntry(mutex,pgdOffset);
	
	//get pmd	
	PageMiddleDirectory * pmd = pud->getEntry(pudOffset);
	if (pmd == NULL)
		pmd = pud->makeNewEntry(mutex,pudOffset);
		
	//get pmd	
	PageTableEntry * pte = pmd->getEntry(pmdOffset);
	if (pte == NULL)
		pte = pmd->makeNewEntry(mutex,pmdOffset);
		
	//get page
	return pte->entries[pteOffset];
}

/*******************  FUNCTION  *********************/
void PageTable::clear(void * baseAddr,size_t size)
{
	//seutp
	uint64_t end = (uint64_t)baseAddr + size;
	uint64_t start = ((uint64_t)baseAddr) & (~NUMAPROF_PAGE_MASK);
	
	//loop
	for (uint64_t addr = start; addr < end ; addr += NUMAPROF_PAGE_SIZE)
		getPage((void*)addr).numaNode = NUMAPROF_DEFAULT_NUMA_NODE;
}

}
