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
void PageTable::setHugePageFromPinnedThread(size_t addr,bool value)
{
	//check
	assert(NUMAPROF_PAGE_LEVEL_ENTRIES * NUMAPROF_PAGE_SIZE == NUMAPROG_HUGE_PAGE_SIZE);
	
	//align on huge pahes
	addr &= ~NUMAPROG_HUGE_PAGE_MASK;
	Page * page = &getPage(addr);
	for (size_t i = 0 ; i < NUMAPROF_PAGE_LEVEL_ENTRIES ; i++)
		page[i].fromPinnedThread = value;
}

/*******************  FUNCTION  *********************/
bool PageTable::canBeHugePage(size_t addr)
{
	//check
	assert(NUMAPROF_PAGE_LEVEL_ENTRIES * NUMAPROF_PAGE_SIZE == NUMAPROG_HUGE_PAGE_SIZE);
	
	//align on huge pahes
	addr &= ~NUMAPROG_HUGE_PAGE_MASK;
	assert(addr % NUMAPROF_PAGE_SIZE == 0);
	
	//get first page
	Page * page = &getPage(addr);
	int fd = page->fd;
	
	//trivial
	if (fd == NUMAPROF_PAGE_UNMAPPED_FD)
	{
		printf("First is unmapped, so k4 page\n");
		return false;
	}
	
	//check if all page have same fd
	for (size_t i = 1 ; i < NUMAPROF_PAGE_LEVEL_ENTRIES ; i++)
		if (page[i].fd != fd)
		{
			printf("fd %d != %d, so k4 page\n",page[i].fd,fd);
			return false;
		}
	
	printf("is huge page\n");
	return true;
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
		page.fromPinnedThread = NUMAPROF_DEFAULT_THREAD_PIN;

		//free mem
		if (page.allocStatus == PAGE_ALLOC_FRAG)
			delete (AllocPointerPageMap*)page.allocPtr;
		
		page.allocPtr = NULL;
		page.allocStatus = PAGE_ALLOC_NONE;
		page.fd = NUMAPROF_PAGE_UNMAPPED_FD;
	}
}

/*******************  FUNCTION  *********************/
void PageTable::trackMMap(size_t base,size_t size,int fd)
{
	//check
	assert(base % NUMAPROF_PAGE_SIZE == 0);
	assert(size % NUMAPROF_PAGE_SIZE == 0);
	
	printf("MMAP %lx [%lu] => %d\n",base,size,fd);
	
	//seutp
	uint64_t end = (uint64_t)base + size;
	uint64_t start = ((uint64_t)base) & (~NUMAPROF_PAGE_MASK);
	
	//loop
	for (uint64_t addr = start; addr < end ; addr += NUMAPROF_PAGE_SIZE)
	{
		Page & page = getPage(addr);
		page.fd = fd;
	}
}

/*******************  FUNCTION  *********************/
void PageTable::regAllocPointerSmall(size_t baseAddr,size_t size,void * value)
{
	//trivial
	if (size == 0)
		return;

	//check
	assert ((baseAddr & ~NUMAPROF_PAGE_MASK) == ((baseAddr + size - 1) & ~NUMAPROF_PAGE_MASK));
	assert(size <= NUMAPROF_PAGE_SIZE);

	//get page
	Page & page = getPage(baseAddr);
	
	//allocate if need
	AllocPointerPageMap * map;
	if (page.allocStatus == PAGE_ALLOC_FRAG)
	{
		map = (AllocPointerPageMap*)page.allocPtr;
	} else if (page.allocStatus == PAGE_ALLOC_NONE) {
		map = new AllocPointerPageMap;
		page.allocStatus = PAGE_ALLOC_FRAG;
		page.allocPtr = map;
	} else {
		printf("Invalid status of page, should be NONE\n");
		return;
	}
	
	//start
	size_t start = (baseAddr & NUMAPROF_PAGE_MASK) / NUMAPROF_ALLOC_GRAIN;
	size_t end = start + (size/ NUMAPROF_ALLOC_GRAIN);
	
	//mark
	for (size_t i = start ; i < end ;  i++)
		map->entries[i] = value;
}

/*******************  FUNCTION  *********************/
void PageTable::regAllocPointer(size_t baseAddr,size_t size,void * value)
{
	//check
	if (baseAddr % NUMAPROF_ALLOC_GRAIN != 0)
		printf("WARNING : allocated bloc not aligned on grain size : %d (get %lu)\n",NUMAPROF_ALLOC_GRAIN,baseAddr % NUMAPROF_ALLOC_GRAIN);
	
	//compute end
	size_t endAddr = baseAddr + size;
	
	//fit in one page
	if (((baseAddr & ~NUMAPROF_PAGE_MASK) == ((endAddr - 1) & ~NUMAPROF_PAGE_MASK)) && size < NUMAPROF_PAGE_SIZE)
	{
		regAllocPointerSmall(baseAddr,size,value);
	} else {
		//first page end
		size_t firstPageEnd = (baseAddr & (~NUMAPROF_PAGE_MASK)) + NUMAPROF_PAGE_SIZE;
		
		//last page start
		size_t endPageStart = endAddr & ~NUMAPROF_PAGE_MASK;
		
		//setup for first page
		regAllocPointerSmall(baseAddr,firstPageEnd - baseAddr,value);
		
		//middle full pages
		for (size_t addr = firstPageEnd ; addr < endPageStart ; addr += NUMAPROF_PAGE_SIZE)
		{
			//get page
			Page & page = getPage(addr);
			
			//check status
			if (page.allocStatus == PAGE_ALLOC_NONE && value != NULL)
			{
				page.allocStatus = PAGE_ALLOC_FULL;
			} else if (page.allocStatus == PAGE_ALLOC_FULL && value == NULL) {
				page.allocStatus = PAGE_ALLOC_NONE;
			} else if (page.allocStatus == PAGE_ALLOC_FRAG && value != NULL) {
				delete (AllocPointerPageMap*)page.allocPtr;
				page.allocStatus = PAGE_ALLOC_FULL;
			} else {
				printf("WARNING : Invalid page status !\n");
				assert(false);
			}
			
			//setup 
			page.allocPtr = value;
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

/*******************  FUNCTION  *********************/
Page::~Page(void)
{
	if (allocStatus == PAGE_ALLOC_FRAG)
	{
		delete (AllocPointerPageMap*)allocPtr;
	}
}

}
