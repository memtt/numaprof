/*****************************************************
             PROJECT  : numlaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include "PageTable.hpp"

/*******************  NAMESPACE  ********************/
namespace numprof
{

/*******************  FUNCTION  *********************/
Page & PageTable::getPage(uint64_t addr)
{
	//get offsets
	int pteOffset = NUMAPROF_PAGE_LEVEL_ID(addr,0);
	int pmdOffset = NUMAPROF_PAGE_LEVEL_ID(addr,1);
	int pudOffset = NUMAPROF_PAGE_LEVEL_ID(addr,2);
	int pgdOffset = NUMAPROF_PAGE_LEVEL_ID(addr,3);
	
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
	if (pmd == NULL)
		pte = pmd->makeNewEntry(mutex,pmdOffset);
		
	//get page
	return pte->entries[pteOffset];
}

/*******************  FUNCTION  *********************/
void PageTable::clear(uint64_t baseAddr,size_t size)
{
	//seutp
	uint64_t end = baseAddr + size;
	baseAddr = baseAddr & (~NUMAPROF_PAGE_MASK);
	
	//loop
	for (uint64_t addr = baseAddr; addr < end ; addr += NUMAPROF_PAGE_SIZE)
		getPage(addr).numaNode = NUMAPROF_DEFAULT_NUMA_NODE;
}

}
