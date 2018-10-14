/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.0-dev
             DATE     : 02/2018
             AUTHOR   : Valat Sébastien
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef CPU_SIMPLE_FLAT_CACHE_CPP
#define CPU_SIMPLE_FLAT_CACHE_CPP

/********************  HEADERS  *********************/
#include "CpuCache.hpp"

/********************  NAMESPACE  *******************/
namespace numaprof
{

/*********************  CLASS  **********************/
/**
 * Implement a simple flat cache for each thread. There is no sharing and no levels.
 * This is a really simple implementation for a first look. 
 * It currently implement a LRU policy.
**/
class CpuFlatCache
{
	public:
		CpuFlatCache(void);
		virtual ~CpuFlatCache(void);
		virtual bool onMemoryAccess(size_t addr);
		virtual void onThreadMove(const CpuBindList & cpuList);
	private:
		size_t size;
		size_t cacheLineSize;
		size_t associativity;
		size_t waySize;
		size_t * tagLineVirtAddress;
};

}

#endif //CPU_SIMPLE_FLAT_CACHE_CPP
