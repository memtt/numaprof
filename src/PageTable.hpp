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
#include "Mutex.hpp"
#include <cstdlib>
#include <cassert>
#include <iostream>

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
		Page & getPage(void * addr);
		void clear(void * baseAddr,size_t size);
	private:
		 PageGlobalDirectory pgd;
		 Mutex mutex;
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
	assert(id >= 0 && id < NUMAPROF_PAGE_LEVEL_ENTRIES);
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
	assert(id >= 0 && id < NUMAPROF_PAGE_LEVEL_ENTRIES);
	return entries[id];
}

}

#endif //PAGE_TABLE_HPP
