/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef CPU_SIMPLE_FLAT_CACHE_HPP
#define CPU_SIMPLE_FLAT_CACHE_HPP

/********************  HEADERS  *********************/
#include <cstdint>
#include "CpuCache.hpp"

/*******************  CONSTS  ***********************/
//Shift in bits to make cache line index
#define SIMPLE_FLAT_CACHE_LINE_SHIFT 6
//most cpus use 64 bytes cache lines.
#define SIMPLE_FLAT_CACHE_LINE_SIZE (1<<SIMPLE_FLAT_CACHE_LINE_SHIFT)

/********************  NAMESPACE  *******************/
namespace numaprof
{

/*********************  TYPES  **********************/
/** Define type size to store LRU aging tag **/
typedef uint8_t SimpleFlatCacheLRUAge;

/*********************  CLASS  **********************/
/**
 * Implement a simple flat cache for each thread. There is no sharing and no levels.
 * This is a really simple implementation for a first look. 
 * It currently implement a LRU policy in a really naïve.
**/
class CpuSimpleFlatCache : public CpuCache
{
	public:
		CpuSimpleFlatCache(size_t size,size_t associativity);
		virtual ~CpuSimpleFlatCache(void);
		virtual bool onMemoryAccess(size_t addr);
		virtual void onThreadMove(const CpuBindList & cpuList);
	private:
		inline size_t getIndex(size_t row,size_t way) const {return row * associativity + way;};
		inline size_t getRow(size_t virtIndex) const { return virtIndex % waySize;};
		inline size_t getVirtIndex(size_t addr) const { return addr >> SIMPLE_FLAT_CACHE_LINE_SHIFT;};
		inline void scaleDownAges(size_t row);
		inline size_t selectVictim(size_t row) const;
	private:
		size_t size;
		size_t associativity;
		size_t waySize;
		size_t * tagLineVirtIndex;
		SimpleFlatCacheLRUAge * tagLineAge;
		SimpleFlatCacheLRUAge * tagCurrentAge;
		SimpleFlatCacheLRUAge * bufSortAges;
};

}

#endif //CPU_SIMPLE_FLAT_CACHE_GPP
