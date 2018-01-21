/*****************************************************
             PROJECT  : numaprof
             VERSION  : 0.0.0-dev
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <cassert>
#include <cstdint>
#include <iostream>
#include "PageTable.hpp"
#include "../common/Debug.hpp"
#include "../portability/OS.hpp"

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
void PageTable::setHugePageFromPinnedThread(size_t addr, int numa,bool pinned)
{
	//check
	assert(NUMAPROF_PAGE_LEVEL_ENTRIES * NUMAPROF_PAGE_SIZE == NUMAPROG_HUGE_PAGE_SIZE);
	
	//align on huge pahes
	addr &= ~NUMAPROG_HUGE_PAGE_MASK;
	Page * page = &getPage(addr);
	for (size_t i = 0 ; i < NUMAPROF_PAGE_LEVEL_ENTRIES ; i++)
	{
		page[i].fromPinnedThread = pinned;
		page[i].canBeHugePage = true;
		page[i].numaNode = numa;
	}
}

/*******************  FUNCTION  *********************/
void PageTable::setHugePageNuma(size_t addr,int numa)
{
	//check
	assert(NUMAPROF_PAGE_LEVEL_ENTRIES * NUMAPROF_PAGE_SIZE == NUMAPROG_HUGE_PAGE_SIZE);
	
	//align on huge pahes
	addr &= ~NUMAPROG_HUGE_PAGE_MASK;
	Page * page = &getPage(addr);
	for (size_t i = 0 ; i < NUMAPROF_PAGE_LEVEL_ENTRIES ; i++)
		page[i].numaNode = numa;
}

/*******************  FUNCTION  *********************/
void PageTable::markHugePage(size_t addr)
{
	//check
	assert(NUMAPROF_PAGE_LEVEL_ENTRIES * NUMAPROF_PAGE_SIZE == NUMAPROG_HUGE_PAGE_SIZE);
	
	//align on huge pahes
	addr &= ~NUMAPROG_HUGE_PAGE_MASK;
	assert(addr % NUMAPROF_PAGE_SIZE == 0);
	
	//get first page
	Page * page = &getPage(addr);
	
	//mark
	for (size_t i = 1 ; i < NUMAPROF_PAGE_LEVEL_ENTRIES ; i++)
		page[i].canBeHugePage = true;
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
		return false;
	
	//check if all page have same fd
	for (size_t i = 1 ; i < NUMAPROF_PAGE_LEVEL_ENTRIES ; i++)
		if (page[i].fd != fd)
			return false;
	
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
		page.canBeHugePage = false;
	}
}

/*******************  FUNCTION  *********************/
void PageTable::trackMMap(size_t base,size_t size,int fd)
{
	//check
	assert(base % NUMAPROF_PAGE_SIZE == 0);
	//assert(size % NUMAPROF_PAGE_SIZE == 0);
	
	//seutp
	uint64_t end = (uint64_t)base + size;
	if ((end & NUMAPROF_PAGE_MASK) != 0)
		end += NUMAPROF_PAGE_SIZE - (end & NUMAPROF_PAGE_MASK);
	uint64_t start = ((uint64_t)base) & (~NUMAPROF_PAGE_MASK);
	
	//loop
	for (uint64_t addr = start; addr < end ; addr += NUMAPROF_PAGE_SIZE)
	{
		Page & page = getPage(addr);
		page.fd = fd;
	}
}

/*******************  FUNCTION  *********************/
void PageTable::mremap(size_t oldAddr,size_t oldSize,size_t newAddr, size_t newSize)
{
	//check
	assert(oldAddr % NUMAPROF_PAGE_SIZE == 0);
	assert(oldSize % NUMAPROF_PAGE_SIZE == 0);
	assert(newAddr % NUMAPROF_PAGE_SIZE == 0);
	assert(newSize % NUMAPROF_PAGE_SIZE == 0);
	
	//compute copy size
	size_t copySize = oldSize;
	if (newSize < copySize)
		copySize = newSize;
	
	//if not overlap
	if (oldAddr + oldSize <= newAddr || newAddr + newSize <= oldAddr)
	{
		for (size_t i = 0 ; i < copySize ; i += NUMAPROF_PAGE_SIZE)
			getPage(newAddr+i).remap(getPage(oldAddr+i));
		clear(oldAddr,oldSize);
	} else {
		Page * buf = new Page[copySize / NUMAPROF_PAGE_SIZE];
		for (size_t i = 0 ; i < copySize ; i += NUMAPROF_PAGE_SIZE)
			buf[i / NUMAPROF_PAGE_SIZE] = getPage(oldAddr+i);
		clear(oldAddr,oldSize);
		for (size_t i = 0 ; i < copySize ; i += NUMAPROF_PAGE_SIZE)
			getPage(newAddr+i).remap(buf[i / NUMAPROF_PAGE_SIZE]);
		delete [] buf;
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
	AllocPointerPageMap * map = NULL;
	if (page.allocStatus == PAGE_ALLOC_FRAG)
	{
		map = (AllocPointerPageMap*)page.allocPtr;
	} else if (page.allocStatus == PAGE_ALLOC_NONE) {
		//there could be thread fighting for this particular case, need to handle safety
		//idea to avoid a single mutex =>  we hash the address to have multiple one
		//to reduce contention
		int mutexId = (baseAddr / NUMAPROG_HUGE_PAGE_SIZE) % NUMAPROF_FRAG_MUTEX_CNT;
		fragMutex[mutexId].lock();
			if (page.allocStatus == PAGE_ALLOC_NONE)
			{
				map = new AllocPointerPageMap;
				page.allocPtr = map;
				page.allocStatus = PAGE_ALLOC_FRAG;
			} else if (page.allocStatus == PAGE_ALLOC_FRAG) {
				map = (AllocPointerPageMap*)page.allocPtr;
			} else {
				numaprofWarning("Must never append !");
				return;
			}
		fragMutex[mutexId].unlock();
	} else if (page.allocStatus == PAGE_ALLOC_FULL) {
		printf("Invalid status of page, should be NONE, get FULL : %p\n",page.allocPtr);
		return;
	} else {
		printf("Invalid status of page, should be NONE, get invalid value\n");
		return;
	}
	
	//start
	size_t start = (baseAddr & NUMAPROF_PAGE_MASK) / NUMAPROF_ALLOC_GRAIN;
	if (size % NUMAPROF_ALLOC_GRAIN != 0)
		size += NUMAPROF_ALLOC_GRAIN;
	size_t end = start + (size/ NUMAPROF_ALLOC_GRAIN);
	
	//mark
	#ifdef NUMAPROF_TRACE_PAGE_TABLE_ALLOCS
		printf("setup map %p from %lu => %lu => %p\n",map,start,end,value);
	#endif
	for (size_t i = start ; i < end ;  i++)
	{
		//#ifndef NDEBUG
			if (value != NULL && map->entries[i] != NULL)
				fprintf(stderr,"NUMAPROF: WARNING: Invalid status, get %lu entry = %p of %p, expect NULL !\n",i,map->entries[i],map);
		//#endif
		map->entries[i] = value;
	}
}

/*******************  FUNCTION  *********************/
void PageTable::regAllocPointer(size_t baseAddr,size_t size,void * value)
{
	//null
	if (baseAddr == 0)
		return;

	//check
	if (baseAddr % NUMAPROF_ALLOC_GRAIN != 0)
		numaprofWarning("WARNING : WARNING: allocated bloc not aligned on grain size : %d (get %lu)\n",NUMAPROF_ALLOC_GRAIN,baseAddr % NUMAPROF_ALLOC_GRAIN);
	
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
		#ifdef NUMAPROF_TRACE_PAGE_TABLE_ALLOCS
			if (firstPageEnd < endPageStart)
				printf("Setup full page %p\n",value);
		#endif
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
				printf("WARNING : Invalid page status %d => %p => %p !\n",page.allocStatus,page.allocPtr,value);
				continue;
			}
			
			//setup 
			page.allocPtr = value;
		}
		
		//setup for last page
		regAllocPointerSmall(endPageStart,endAddr - endPageStart,value);
	}
}

/*******************  FUNCTION  *********************/
void PageTable::onMbind(void * addr,size_t size,bool pinned)
{
	//seutp
	uint64_t end = (uint64_t)addr + size;
	uint64_t start = ((uint64_t)addr) & (~NUMAPROF_PAGE_MASK);
	
	//loop 
	for (size_t cur = start ; cur < end ; cur += NUMAPROF_PAGE_SIZE)
	{
		//get page
		Page & page = getPage(cur);
		
		//update mapping
		page.numaNode = OS::getNumaOfPage(cur);
		
		//update pinning
		page.fromPinnedThread = pinned;
	}
}

/*******************  FUNCTION  *********************/
void PageTable::markObjectFileAsNotPinned(void * addr,size_t size)
{
	//seutp
	uint64_t end = (uint64_t)addr + size;
	uint64_t start = ((uint64_t)addr) & (~NUMAPROF_PAGE_MASK);
	
	//loop 
	for (size_t cur = start ; cur < end ; cur += NUMAPROF_PAGE_SIZE)
	{
		//get page
		Page & page = getPage(cur);
		
		//update pinning
		if (page.numaNode < 0)
			page.fromPinnedThread = false;
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
