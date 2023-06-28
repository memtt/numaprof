/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef PAGE_TABLE_HPP
#define PAGE_TABLE_HPP

/********************  HEADERS  *********************/
#include "../portability/Mutex.hpp"
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <cstring>

/*******************  NAMESPACE  ********************/
namespace numaprof
{
	
/********************  MACROS  **********************/
/** Page offset, 12 correspond to 4K **/
#define NUMAPROF_PAGE_OFFSET 12
/** Page size, with offset 12 it represent 4K **/
#define NUMAPROF_PAGE_SIZE (1lu << NUMAPROF_PAGE_OFFSET)
/** Mask to get the offset in a page **/
#define NUMAPROF_PAGE_MASK (NUMAPROF_PAGE_SIZE - 1)
/** Number of bit for every level of the page table, 9 correspond to 512 entries. **/
#define NUMAPROF_PAGE_LEVEL_BITS 9
/** Mask to get the offset into one level **/
#define NUMAPROF_PAGE_LEVEL_MASK ((1lu << NUMAPROF_PAGE_LEVEL_BITS) - 1)
/** Number of entries into one page table level (512) **/
#define NUMAPROF_PAGE_LEVEL_ENTRIES (1lu <<NUMAPROF_PAGE_LEVEL_BITS)
/** Shift to move the mask onto its location to be applied to a given level **/
#define NUMAPROF_PAGE_LEVEL_SHIFT(level) (NUMAPROF_PAGE_OFFSET + NUMAPROF_PAGE_LEVEL_BITS * level)
/** Get the index for a given level for the given address **/
#define NUMAPROF_PAGE_LEVEL_ID(ptr,level) ((ptr & (NUMAPROF_PAGE_LEVEL_MASK << NUMAPROF_PAGE_LEVEL_SHIFT(level))) >> NUMAPROF_PAGE_LEVEL_SHIFT(level))
/** Size of a huge page : 2M **/
#define NUMAPROG_HUGE_PAGE_SIZE (2*1024*1024)
/** Mask for huge pages **/
#define NUMAPROF_HUGE_PAGE_MASK ((2*1024*1024)-1)
/** Default value for the page NUMA node to say UNKNOWN **/
#define NUMAPROF_DEFAULT_NUMA_NODE (-2)
/** 
 * Minimal allocation considered for the allocation show table
 * If the allocator allocate smaller segments we might
 * habe memory leaks and miss-count the per allocation counters.
**/
#define NUMAPROF_ALLOC_GRAIN 8
/**
 * If a page was already present before the first touch how to consider it
 * form the pinning point of view. True to consider it pinned.
**/
#define NUMAPROF_DEFAULT_THREAD_PIN true
/**
 * Define if the page append to a file descriptor.
 * -200 is default values.
**/
#define NUMAPROF_PAGE_UNMAPPED_FD -200
/**
 * Anonimous page allocation has a page FD setup to -1
**/
#define NUMAPROF_PAGE_ANON_FD -1
/**
 * Number of mutex to protect the alloc map entries.
 * To not store a mutex on every entry we allocate a certain number
 * of static mutexes in the main struct and distribute the access
 * depending on a modulo on the address. This value define
 * how many mutex we allocate.
**/
#define NUMAPROF_FRAG_MUTEX_CNT 128
//#define NUMAPROF_TRACE_PAGE_TABLE_ALLOCS

/********************  ENUM  ************************/
/**
 * Status of a page for the allocation status.
**/
enum PageAllocStatus
{
	/** There is no allocation on the page **/
	PAGE_ALLOC_NONE,
	/** 
	 * The allocation cover more than the page so the pointer
	 * directly give the allocation descriptor 
	**/
	PAGE_ALLOC_FULL,
	/**
	 * The page contain multiple small allocation so it is fragmented.
	**/
	PAGE_ALLOC_FRAG
};

/********************  STRUCT  **********************/
/**
 * Structure to be used to keep track of the allocated segement when
 * a page is fragmented in multiple small allocations.
 * We allocate this structure to cover a huge page.
**/
struct AllocPointerPageMap
{
	AllocPointerPageMap(void) {memset(entries,0,sizeof(entries));};
	void * entries[NUMAPROF_PAGE_SIZE/NUMAPROF_ALLOC_GRAIN];
};

/********************  STRUCT  **********************/
/**
 * Define a page to build the page table and Options.
**/
struct Page
{
	Page(void) {numaNode = NUMAPROF_DEFAULT_NUMA_NODE; fromPinnedThread = NUMAPROF_DEFAULT_THREAD_PIN;allocStatus = PAGE_ALLOC_NONE;allocPtr = NULL;fd = NUMAPROF_PAGE_UNMAPPED_FD;canBeHugePage = false;};
	~Page(void);
	inline void remap(const Page & page);
	void * getAllocPointer(size_t addr);
	//vars
	/** Defnie the NUMA node of the page of -2 if the page is not yet allocated nor first touched **/
	int numaNode;
	//@todo optimized by merging with fromPinnedThread using bitfields
	/** File descriptor of the page, this is used by huge page detection system. **/
	int fd;
	/** Define the the page has been first touched by a pinned of not pinned thread. **/
	bool fromPinnedThread;
	/** Keep track of the huge page status. **/
	bool canBeHugePage;
	/** Status of the page (fragmented or not) concerning the allocations **/
	PageAllocStatus allocStatus;
	/** Pointer to the allocation state of the fragment descriptor **/
	void * allocPtr;
};

/*********************  CLASS  **********************/
/**
 * Define a page table level. It uses template so we can embedded easily
 * a sub-page table into the parrent one and build the full tree with
 * multiple levels.
**/
template <class T>
class PageTableLevel
{
	public:
		PageTableLevel(void);
		~PageTableLevel(void);
		T * makeNewEntry(Mutex & mutex,int id);
		T * getEntry(int id);
	private:
		/** List of entries to reach the sub-level **/
		T * entries[NUMAPROF_PAGE_LEVEL_ENTRIES];
};

/********************  STRUCT  **********************/
/**
 * Lower level of the page table, if contain directly a list of page.
**/
struct PageTableEntry
{
	/** List of page of the last level of the page table. **/
	Page entries[NUMAPROF_PAGE_LEVEL_ENTRIES];
};

/********************  STRUCT  **********************/
/** Build the sub levels of the page table **/
class PageMiddleDirectory : public PageTableLevel<PageTableEntry> {};
/** Build the sub levels of the page table **/
class PageUpperDirectory : public PageTableLevel<PageMiddleDirectory> {};
/** Build the sub levels of the page table **/
class PageGlobalDirectory : public PageTableLevel<PageUpperDirectory> {};

/*********************  CLASS  **********************/
/**
 * Object used to handle the show page table to keep track of page mapping.
**/
class PageTable
{
	public:
		Page & getPage(size_t addr);
		bool canBeHugePage(size_t addr);
		void markHugePage(size_t addr);
		void trackMMap(size_t base,size_t size,int fd);
		void clear(size_t baseAddr,size_t size);
		void mremap(size_t oldAddr,size_t oldSize,size_t newAddr, size_t newSize);
		void regAllocPointer(size_t baseAddr,size_t size,void * value);
		void freeAllocPointer(size_t baseAddr,size_t size,void * value);
		void setHugePageFromPinnedThread(size_t addr, int numa,bool pinned);
		void setHugePageNuma(size_t addr,int numa);
		void onMbind(void * addr,size_t size,bool pinned);
		void markObjectFileAsNotPinned(void * addr,size_t size);
	private:
		void regAllocPointerSmall(size_t baseAddr,size_t size,void * value);
	private:
		/** Root of the page table **/
		PageGlobalDirectory pgd;
		/** Mutex to protect the page table **/
		Mutex mutex;
		/** Mutex to protect the fragmented allocation entries **/
		Mutex fragMutex[NUMAPROF_FRAG_MUTEX_CNT];
};

/*******************  FUNCTION  *********************/
/**
 * Init the entries of a page level, by default they are all NULL.
**/
template <class T>
PageTableLevel<T>::PageTableLevel(void)
{
	for (size_t i = 0 ; i < NUMAPROF_PAGE_LEVEL_ENTRIES ; i++)
		entries[i] = NULL;
}

/*******************  FUNCTION  *********************/
/**
 * Destructor of the sub page table levels. Free the related memory.
**/
template <class T>
PageTableLevel<T>::~PageTableLevel(void)
{
	for (size_t i = 0 ; i < NUMAPROF_PAGE_LEVEL_ENTRIES ; i++)
	{
		if (entries[i] != NULL)
			delete entries[i];
	}
}

/*******************  FUNCTION  *********************/
/**
 * Allocate a new entry in the page table level. Caution,
 * this operation take a local spinlock.
**/
template <class T>
T * PageTableLevel<T>::makeNewEntry(Mutex & mutex,int id)
{
	assert(id >= 0 && (size_t)id < NUMAPROF_PAGE_LEVEL_ENTRIES);
	if (entries[id] == NULL)
	{
		mutex.lock();
		if (entries[id] == NULL)
		{
			//std::cout << "Allocate " << id << std::endl;
			entries[id] = new T;
		}
		mutex.unlock();
	}
	return entries[id];
}

/*******************  FUNCTION  *********************/
/**
 * Return a given entry in the page table leve. This is mainly to add
 * assetion check in debug mode.
**/
template <class T>
T * PageTableLevel<T>::getEntry(int id)
{
	assert(id >= 0 && (size_t)id < NUMAPROF_PAGE_LEVEL_ENTRIES);
	return entries[id];
}

/*******************  FUNCTION  *********************/
/**
 * Return the allocation pointer to descrive the allocation
 * covered by the page. It take care the the current status :
 * NONE, FULL or fragmented.
**/
inline void * Page::getAllocPointer(size_t addr)
{
	if (allocStatus == PAGE_ALLOC_NONE)
	{
		return NULL;
	} else if (allocStatus == PAGE_ALLOC_FULL) {
		return allocPtr;
	} else if (allocStatus == PAGE_ALLOC_FRAG) {
		size_t offset = addr & NUMAPROF_PAGE_MASK;
		size_t index = offset / NUMAPROF_ALLOC_GRAIN;
		AllocPointerPageMap * map = (AllocPointerPageMap*)allocPtr;
		return map->entries[index];
	} else {
		assert(false);
		return NULL;
	}
}

/*******************  FUNCTION  *********************/
/**
 * Remap a page yb copying the internal values.
**/
inline void Page::remap(const Page & page)
{
	fd = page.fd;
	fromPinnedThread = page.fromPinnedThread;
	canBeHugePage = page.canBeHugePage;
	numaNode = page.numaNode;
}

}

#endif //PAGE_TABLE_HPP
