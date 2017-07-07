/*****************************************************
             PROJECT  : numlaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef PAGE_TABLE_HPP
#define PAGE_TABLE_HPP

/********************  HEADERS  *********************/
#include <mutex>
#include <cstdlib>
#include <cassert>
#include <cstdint>

/*******************  NAMESPACE  ********************/
namespace numprof
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
#define NUMAPROF_DEFAULT_NUMA_NODE (-2)

/********************  STRUCT  **********************/
struct Page
{
	Page(void) {numaNode = NUMAPROF_DEFAULT_NUMA_NODE;};
	int numaNode;
};

/*********************  CLASS  **********************/
template <class T>
class PageTableLevel
{
	public:
		PageTableLevel(void);
		~PageTableLevel(void);
		T * makeNewEntry(std::mutex & mutex,int id);
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
		Page & getPage(uint64_t addr);
		void clear(uint64_t baseAddr,size_t size);
	private:
		 PageGlobalDirectory pgd;
		 std::mutex mutex;
};

/*******************  FUNCTION  *********************/
template <class T>
PageTableLevel<T>::PageTableLevel(void)
{
	for (int i = 0 ; i < NUMAPROF_PAGE_LEVEL_ENTRIES ; i++)
		entries[i] = NULL;
}

/*******************  FUNCTION  *********************/
template <class T>
PageTableLevel<T>::~PageTableLevel(void)
{
	for (int i = 0 ; i < NUMAPROF_PAGE_LEVEL_ENTRIES ; i++)
	{
		if (entries[i] != NULL)
			delete entries[i];
	}
}

/*******************  FUNCTION  *********************/
template <class T>
T * PageTableLevel<T>::makeNewEntry(std::mutex & mutex,int id)
{
	assert(id >= 0 && id < NUMAPROF_PAGE_LEVEL_ENTRIES);
	if (entries[id] == NULL)
	{
		mutex.lock();
		if (entries[id] == NULL)
			entries[id] = new T;
		mutex.unlock();
	}
	return entries[id];
}

/*******************  FUNCTION  *********************/
template <class T>
T * PageTableLevel<T>::getEntry(int id)
{
	assert(id >= 0 && id < NUMAPROF_PAGE_LEVEL_ENTRIES);
	return entries[id];
}

}

#endif //PAGE_TABLE_HPP
