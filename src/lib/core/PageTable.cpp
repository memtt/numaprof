/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
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
/**
 * Get the page containing the given address for the shadow page table.
 * @param addr Define the address for which we want the page.
**/
Page & PageTable::getPage(size_t addr)
{
	//convert
	uint64_t ptr = (uint64_t)addr;

	//get offsets
	int pteOffset = NUMAPROF_PAGE_LEVEL_ID(ptr,0);
	int pmdOffset = NUMAPROF_PAGE_LEVEL_ID(ptr,1);
	int pudOffset = NUMAPROF_PAGE_LEVEL_ID(ptr,2);
	int pgdOffset = NUMAPROF_PAGE_LEVEL_ID(ptr,3);
	
	//debug
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
/**
 * Mark the pages as huge page and setup its pinned and numa attribute.
 * @param addr On address contained by the huge page.
 * @param numa NUMA node of the huge page.
 * @param pinned Defined if the huge page is first touched by a pinned of non pinned thread.
**/
void PageTable::setHugePageFromPinnedThread(size_t addr, int numa,bool pinned)
{
	//check
	assert(NUMAPROF_PAGE_LEVEL_ENTRIES * NUMAPROF_PAGE_SIZE == NUMAPROG_HUGE_PAGE_SIZE);
	
	//align on huge pahes
	addr &= ~NUMAPROF_HUGE_PAGE_MASK;
	Page * page = &getPage(addr);
	for (size_t i = 0 ; i < NUMAPROF_PAGE_LEVEL_ENTRIES ; i++)
	{
		page[i].fromPinnedThread = pinned;
		page[i].canBeHugePage = true;
		page[i].numaNode = numa;
	}
}

/*******************  FUNCTION  *********************/
/**
 * Set the NUMA node of a huge page.
 * @param addr One address inside the huge page.
 * @param numa The NUMA node of the huge page.
**/
void PageTable::setHugePageNuma(size_t addr,int numa)
{
	//check
	assert(NUMAPROF_PAGE_LEVEL_ENTRIES * NUMAPROF_PAGE_SIZE == NUMAPROG_HUGE_PAGE_SIZE);
	
	//align on huge pahes
	addr &= ~NUMAPROF_HUGE_PAGE_MASK;
	Page * page = &getPage(addr);
	for (size_t i = 0 ; i < NUMAPROF_PAGE_LEVEL_ENTRIES ; i++)
		page[i].numaNode = numa;
}

/*******************  FUNCTION  *********************/
/**
 * Mark the given address as a huge page and all the contained pages.
**/
void PageTable::markHugePage(size_t addr)
{
	//check
	assert(NUMAPROF_PAGE_LEVEL_ENTRIES * NUMAPROF_PAGE_SIZE == NUMAPROG_HUGE_PAGE_SIZE);
	
	//align on huge pahes
	addr &= ~NUMAPROF_HUGE_PAGE_MASK;
	assert(addr % NUMAPROF_PAGE_SIZE == 0);
	
	//get first page
	Page * page = &getPage(addr);
	
	//mark
	for (size_t i = 1 ; i < NUMAPROF_PAGE_LEVEL_ENTRIES ; i++)
		page[i].canBeHugePage = true;
}

/*******************  FUNCTION  *********************/
/**
 * Check if the page can be a huge page based on the file descriptor checking.
 * A huge page cannot be allocated between two file descriptors.
 * At least this is what we consider here.
**/
bool PageTable::canBeHugePage(size_t addr)
{
	//check
	assert(NUMAPROF_PAGE_LEVEL_ENTRIES * NUMAPROF_PAGE_SIZE == NUMAPROG_HUGE_PAGE_SIZE);
	
	//align on huge pahes
	addr &= ~NUMAPROF_HUGE_PAGE_MASK;
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
/**
 * Clear the given segemnt. It will reset content of all the inner pages.
 * @param baseAddr base address of the segement to clear.
 * @param size Size of the segement to clear.
**/
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
/**
 * Function to call when mmap is used to setup new address range.
 * @param base Base address of the new allocated segment.
 * @param size Size of the segement.
 * @param fd File descriptor to track huge pages.
**/
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
/**
 * Remap a segement by moving the pages.
 * @param oldAddr Original base address of the segment.
 * @param oldSize Old size of the segement.
 * @param newAddr New base address of the segment.
 * @param newSize New size of the segement.
**/
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
/**
 * Register an allocation pointer into the converned pages.
 * It build the fragmented sub container as the segment do
 * not fully cover the page.
 * @param baseAddr Base address of the allocated segement.
 * @param size Size of the segement.
 * @param value Custom pointer to attach to the allocated segement.
**/
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
		int mutexId = (baseAddr / NUMAPROF_PAGE_SIZE) % NUMAPROF_FRAG_MUTEX_CNT;
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
				fprintf(stderr,"NUMAPROF: WARNING: Invalid status, get %lu entry = %p of %p, expect NULL !\n",i,map->entries[i],(void*)map);
		//#endif
		map->entries[i] = value;
	}
}

/*******************  FUNCTION  *********************/
/**
 * Register an allocation pointer into the converned pages.
 * It can build the fragmented sub container as the segment do
 * not fully cover the page.
 * @param baseAddr Base address of the allocated segement.
 * @param size Size of the segement.
 * @param value Custom pointer to attach to the allocated segement.
**/
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
				printf("WARNING : Invalid page status %d => %p => %p ! This migt introduce biases no malloc call site metrics\n",page.allocStatus,page.allocPtr,value);
				if (baseAddr % 8 != 0 || size % 8 != 0)
					printf("WARNING: This append because you are using allocation size or addresses non multiple of 8 (addr%%8=%lu, size%%8=%lu)\n",baseAddr % 8, size % 8);
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
/**
 * On mmbind call we need to update the NUMA mapping status of the concerned pages.
 * @param addr Base address of the related segement.
 * @param size Size of the segemnt.
 * @param pinned Define if marked as pinned or not. It depend on the pinning mask provided to mbind.
**/
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
/**
 * If option is enabled we need to mark the binary object files as not pinned.
 * @param addr Base address of the binary obejct in memory.
 * @param size Size of the binary obect.
**/
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
/**
 * Free the information assigned to allocated pointers.
 * @param baseAddress Base address of the segment.
 * @param size Size of the segment.
 * @param value Old value to check when removing (not used).
**/
void PageTable::freeAllocPointer(size_t baseAddr,size_t size,void * value)
{
	regAllocPointer(baseAddr,size,NULL);
}

/*******************  FUNCTION  *********************/
/**
 * If the page is fragemented free the allocation fragments.
**/
Page::~Page(void)
{
	if (allocStatus == PAGE_ALLOC_FRAG)
	{
		delete (AllocPointerPageMap*)allocPtr;
	}
}

}
