/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef CPU_SIMPLE_FLAT_CACHE_STATIC_HPP
#define CPU_SIMPLE_FLAT_CACHE_STATIC_HPP

/********************  HEADERS  *********************/
#include <cstdint>
#include "CpuSimpleFlatCache.hpp"

/********************  NAMESPACE  *******************/
namespace numaprof
{

/*********************  TYPES  **********************/
/** Define type size to store LRU aging tag **/
typedef uint8_t SimpleFlatCacheStaticLRUAge;

/*********************  CLASS  **********************/
/**
 * Implement a simple flat cache for each thread. There is no sharing and no levels.
 * This is a really simple implementation for a first look. 
 * It currently implement a LRU policy in a really naïve.
 * This is same implementation than CpuSimpleFlatCache but in a static template
 * based way which is less configuration but might be faster.
**/
template <int waySize,int associativity>
class CpuSimpleFlatCacheStatic : public CpuCache
{
	public:
		CpuSimpleFlatCacheStatic(void);
		virtual ~CpuSimpleFlatCacheStatic(void);
		virtual bool onMemoryAccess(size_t addr);
		inline bool onMemoryAccessInlined(size_t addr);
		virtual void onThreadMove(const CpuBindList & cpuList);
	private:
		inline size_t getIndex(size_t row,size_t way) const {return row * associativity + way;};
		inline size_t getRow(size_t virtIndex) const { return virtIndex % waySize;};
		inline size_t getVirtIndex(size_t addr) const { return addr >> SIMPLE_FLAT_CACHE_LINE_SHIFT;};
		inline void scaleDownAges(size_t row);
		inline size_t selectVictim(size_t row) const;
	private:
		size_t tagLineVirtIndex[waySize][associativity];
		SimpleFlatCacheLRUAge tagLineAge[waySize][associativity];
		SimpleFlatCacheLRUAge tagCurrentAge[waySize];
		SimpleFlatCacheLRUAge bufSortAges[associativity];
};

}

//include implementation
#include "CpuSimpleFlatCacheStatic_impl.hpp"

#endif //CPU_SIMPLE_FLAT_CACHE_STATIC_HPP
