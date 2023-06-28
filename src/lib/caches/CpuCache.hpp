/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef CPU_CACHE_HPP
#define CPU_CACHE_HPP

/********************  HEADERS  *********************/
#include <stdint.h>
#include "../portability/NumaTopo.hpp"

/********************  NAMESPACE  *******************/
namespace numaprof
{

/*********************  CLASS  **********************/
/**
 * Abstract definition of a cache to be used by the ThreadTracker to register
 * accesses and check presence in cache.
**/
class CpuCache
{
	public:
		CpuCache(void);
		virtual ~CpuCache(void);
		/**
		 * Handler to be called on memory access to check if the data is in the cache
		 * and to register it in if the cache keep it for later access.
		 * @param addr Define the address we want to access. This will automatically
		 * be considered as a cache line access.
		 * @return Return true if the data is already in the cache, false otherwise.
		 * 
		 * @TODO without the size parameter we cannot distinguish large accesses like
		 * large SIMD accseses or accessing crossing caches lines due to non ideal
		 * alignement.
		**/
		virtual bool onMemoryAccess(size_t addr) = 0;
		/**
		 * Notify from thread movement to possibly handle multiple level and topology
		 * of cache.
		 * @param cpuList contain the CPU list where the thread can run on. The cache
		 * need to decide how it interpret it.
		**/
		virtual void onThreadMove(const CpuBindList & cpuList) = 0;
};

}

#endif //CPU_CACHE_HPP
