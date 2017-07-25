/*****************************************************
			 PROJECT  : numaprof
			 VERSION  : 2.3.0
			 DATE     : 05/2017
			 AUTHOR   : Valat Sébastien - CERN
			 LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <cassert>
#include <cstdint>
#include <iostream>
#include "PageTable.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
Page & PageTable::getPage(size_t addr)
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
void PageTable::clear(size_t baseAddr,size_t size)
{
	//seutp
	uint64_t end = (uint64_t)baseAddr + size;
	uint64_t start = ((uint64_t)baseAddr) & (~NUMAPROF_PAGE_MASK);
	
	//loop
	for (uint64_t addr = start; addr < end ; addr += NUMAPROF_PAGE_SIZE)
	{
		Page & page = getPage(addr);
		page.numaNode = NUMAPROF_DEFAULT_NUMA_NODE;
		page.fromPinnedThread = false;
	}
}

/*******************  FUNCTION  *********************/
void regAllocPointerSmall(size_t baseAddr,size_t size,void * value)
{
	//check
	assert (baseAddr & NUMAPROF_PAGE_MASK == endAddr & NUMAPROF_PAGE_MASK);

	//get page
	Page & page = getPage(baseAddr);
	
	//allocate if need
	AllocPointerPageMap * map;
	if (page->allocStatus == PAGE_ALLOC_FRAG)
		map = page->allocPtr;
	else if (page->allocStatus == PAGE_ALLOC_NONE)
		map = (void*)new AllocPointerPageMap;
	else
		printf("Invalid status of page, should be NONE\n");
		
	//store
	page->allocPtr = map;
	
	//start
	size_t start = (baseAddr & NUMAPROF_PAGE_LEVEL_MASK) / NUMAPROF_ALLOC_GRAIN;
	size_t end = ((baseAddr+size) & NUMAPROF_PAGE_LEVEL_MASK) / NUMAPROF_ALLOC_GRAIN;
	
	//mark
	for (size_t i = start ; i < end ;  i++)
		map->entries[i] = value;
}

/*******************  FUNCTION  *********************/
void PageTable::regAllocPointer(size_t baseAddr,size_t size,void * value)
{
	//check
	if (baseAddr % NUMAPROF_ALLOC_GRAIN != 0)
		printf("WARNING : allocated bloc not aligned on grain size : %d (get %d)\n",NUMAPROF_ALLOC_GRAIN,baseAddr % NUMAPROF_ALLOC_GRAIN);
	
	//compute end
	size_t endAddr = baseAddr + size;
	
	//fit in one page
	if (baseAddr & NUMAPROF_PAGE_MASK == endAddr & NUMAPROF_PAGE_MASK && size != NUMAPROF_PAGE_SIZE)
	{
		regAllocPointerSmall(baseAddr,size,value)
	} else {
		//first page end
		size_t firstPageEnd = baseAddr & NUMAPROF_PAGE_MASK + NUMAPROF_PAGE_SIZE;
		
		//last page start
		size_t endPageStart = endAddr & NUMAPROF_PAGE_MASK;
		
		//setup for first page
		regAllocPointerSmall(baseAddr,firstPageEnd - baseAddr,value);
		
		//middle full pages
		for (size_t addr = firstPageEnd ; addr < endPageStart ; addr == NUMAPROF_PAGE_SIZE)
		{
			//get page
			Page & page = getPage(baseAddr);
			
			//check status
			if (page->allocStatus == PAGE_ALLOC_NONE && value != NULL)
			{
				page->allocStatus = PAGE_ALLOC_FULL;
			} else if (page->allocStatus == PAGE_ALLOC_FULL && value == NULL) {
				page->allocStatus = PAGE_ALLOC_NONE;
			} else {
				printf("WARNING : Invalid page status !\n");
			}
			
			//setup 
			page->allocPtr = value;
		}
		
		//setup for last page
		regAllocPointerSmall(endPageStart,endAddr - endPageStart,value);
	}
}

/*******************  FUNCTION  *********************/
void PageTable::freeAllocPointer(size_t baseAddr,size_t size,void * value)
{
	regAllocPointer(baseAddr,size,NULL);
}

}
