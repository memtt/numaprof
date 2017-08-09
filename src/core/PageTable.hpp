/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
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
#define NUMAPROF_PAGE_OFFSET 12
#define NUMAPROF_PAGE_SIZE (1lu << NUMAPROF_PAGE_OFFSET)
#define NUMAPROF_PAGE_MASK (NUMAPROF_PAGE_SIZE - 1)
#define NUMAPROF_PAGE_LEVEL_BITS 9
#define NUMAPROF_PAGE_LEVEL_MASK ((1lu << NUMAPROF_PAGE_LEVEL_BITS) - 1)
#define NUMAPROF_PAGE_LEVEL_ENTRIES (1lu <<NUMAPROF_PAGE_LEVEL_BITS)
#define NUMAPROF_PAGE_LEVEL_SHIFT(level) (NUMAPROF_PAGE_OFFSET + NUMAPROF_PAGE_LEVEL_BITS * level)
#define NUMAPROF_PAGE_LEVEL_ID(ptr,level) ((ptr & (NUMAPROF_PAGE_LEVEL_MASK << NUMAPROF_PAGE_LEVEL_SHIFT(level))) >> NUMAPROF_PAGE_LEVEL_SHIFT(level))
#define NUMAPROG_HUGE_PAGE_SIZE (2*1024*1024)
#define NUMAPROG_HUGE_PAGE_MASK ((2*1024*1024)-1)
#define NUMAPROF_DEFAULT_NUMA_NODE (-2)
#define NUMAPROF_ALLOC_GRAIN 8
#define NUMAPROF_DEFAULT_THREAD_PIN true
#define NUMAPROF_PAGE_UNMAPPED_FD -200
#define NUMAPROF_PAGE_ANON_FD -1
#define NUMAPROF_FRAG_MUTEX_CNT 32
//#define NUMAPROF_TRACE_PAGE_TABLE_ALLOCS

/********************  ENUM  ************************/
enum PageAllocStatus
{
	PAGE_ALLOC_NONE,
	PAGE_ALLOC_FULL,
	PAGE_ALLOC_FRAG
};

/********************  STRUCT  **********************/
struct AllocPointerPageMap
{
	AllocPointerPageMap(void) {memset(entries,0,sizeof(entries));};
	void * entries[NUMAPROF_PAGE_SIZE/NUMAPROF_ALLOC_GRAIN];
};

/********************  STRUCT  **********************/
struct Page
{
	Page(void) {numaNode = NUMAPROF_DEFAULT_NUMA_NODE; fromPinnedThread = NUMAPROF_DEFAULT_THREAD_PIN;allocStatus = PAGE_ALLOC_NONE;allocPtr = NULL;fd = NUMAPROF_PAGE_UNMAPPED_FD;canBeHugePage = false;};
	~Page(void);
	inline void remap(const Page & page);
	void * getAllocPointer(size_t addr);
	//vars
	int numaNode;
	//@todo optimized by merging with fromPinnedThread using bitfields
	int fd;
	bool fromPinnedThread;
	bool canBeHugePage;
	PageAllocStatus allocStatus;
	void * allocPtr;
};

/*********************  CLASS  **********************/
template <class T>
class PageTableLevel
{
	public:
		PageTableLevel(void);
		~PageTableLevel(void);
		T * makeNewEntry(Mutex & mutex,int id);
		T * getEntry(int id);
	private:
		T * entries[NUMAPROF_PAGE_LEVEL_ENTRIES];
};

/********************  STRUCT  **********************/
struct PageTableEntry
{
	Page entries[NUMAPROF_PAGE_LEVEL_ENTRIES];
};

/********************  STRUCT  **********************/
class PageMiddleDirectory : public PageTableLevel<PageTableEntry> {};
class PageUpperDirectory : public PageTableLevel<PageMiddleDirectory> {};
class PageGlobalDirectory : public PageTableLevel<PageUpperDirectory> {};

/*********************  CLASS  **********************/
class PageTable
{
	public:
		Page & getPage(size_t addr);
		bool canBeHugePage(size_t addr);
		void trackMMap(size_t base,size_t size,int fd);
		void clear(size_t baseAddr,size_t size);
		void mremap(size_t oldAddr,size_t oldSize,size_t newAddr, size_t newSize);
		void regAllocPointer(size_t baseAddr,size_t size,void * value);
		void freeAllocPointer(size_t baseAddr,size_t size,void * value);
		void setHugePageFromPinnedThread(size_t addr,bool value);
		void setHugePageNuma(size_t addr,int numa);
	private:
		void regAllocPointerSmall(size_t baseAddr,size_t size,void * value);
	private:
		 PageGlobalDirectory pgd;
		 Mutex mutex;
		 Mutex fragMutex[NUMAPROF_FRAG_MUTEX_CNT];
};

/*******************  FUNCTION  *********************/
template <class T>
PageTableLevel<T>::PageTableLevel(void)
{
	for (size_t i = 0 ; i < NUMAPROF_PAGE_LEVEL_ENTRIES ; i++)
		entries[i] = NULL;
}

/*******************  FUNCTION  *********************/
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
template <class T>
T * PageTableLevel<T>::getEntry(int id)
{
	assert(id >= 0 && (size_t)id < NUMAPROF_PAGE_LEVEL_ENTRIES);
	return entries[id];
}

/*******************  FUNCTION  *********************/
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
inline void Page::remap(const Page & page)
{
	fd = page.fd;
	fromPinnedThread = page.fromPinnedThread;
	canBeHugePage = page.canBeHugePage;
	numaNode = page.numaNode;
}

}

#endif //PAGE_TABLE_HPP
