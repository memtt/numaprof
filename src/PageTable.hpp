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

/*******************  NAMESPACE  ********************/
namespace numprof
{
	
/********************  MACROS  **********************/
#define NUMAPROF_PAGE_LEVEL_BITS 9
#define NUMAPROG_PAGE_LEVEL_ENTRIES (1<<NUMAPROF_PAGE_LEVEL_BITS)

/********************  STRUCT  **********************/
struct Page
{
	Page(void) {numaNode = -2;};
	int numaNode;
};

/*********************  CLASS  **********************/
template <class T>
class PageTableLevel
{
	public:
		PageTableLevel(void);
		~PageTableLevel(void);
		void makeNewEntry(std::mutex & mutex,int id);
		T * getEntry(int id);
		T * entries[NUMAPROG_PAGE_LEVEL_ENTRIES];
};

/********************  STRUCT  **********************/
class PageTableEntry : public PageTableLevel<Page> {};
class PageDirectory : public PageTableLevel<PageTableEntry> {};
class PageUpperDirectory : public PageTableLevel<PageDirectory> {};
class PageMiddleDirectory : public PageTableLevel<PageUpperDirectory> {};
class PageGlobalDirectory : public PageTableLevel<PageMiddleDirectory> {};

/*********************  CLASS  **********************/
class PageTable
{
	public:
		Page & getPage(void * addr);
		void clear(void * baseAddr,size_t size);
	private:
		 PageGlobalDirectory pgd;
		 std::mutex mutex;
};

/*******************  FUNCTION  *********************/
template <class T>
PageTableLevel<T>::PageTableLevel(void)
{
	for (int i = 0 ; i < NUMAPROG_PAGE_LEVEL_ENTRIES ; i++)
		entries[i] = NULL;
}

/*******************  FUNCTION  *********************/
template <class T>
PageTableLevel<T>::~PageTableLevel(void)
{
	for (int i = 0 ; i < NUMAPROG_PAGE_LEVEL_ENTRIES ; i++)
	{
		if (entries[i] != NULL)
			delete entries[i];
	}
}

/*******************  FUNCTION  *********************/
template <class T>
void PageTableLevel<T>::makeNewEntry(std::mutex & mutex,int id)
{
	assert(id >= 0 && id < NUMAPROG_PAGE_LEVEL_ENTRIES);
	if (entries[id] == NULL)
	{
		mutex.lock();
		if (entries[id] == NULL)
			entries[id] = new T;
		mutex.unlock();
	}
}

/*******************  FUNCTION  *********************/
template <class T>
T * PageTableLevel<T>::getEntry(int id)
{
	assert(id >= 0 && id < NUMAPROG_PAGE_LEVEL_ENTRIES);
	return entries[id];
}

}

#endif //PAGE_TABLE_HPP
