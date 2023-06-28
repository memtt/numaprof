/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef CPU_SIMPLE_FLAT_CACHE_STATIC_IMPL_HPP
#define CPU_SIMPLE_FLAT_CACHE_STATIC_IMPL_HPP

/*******************  HEADERS  **********************/
#include <cassert>
#include <algorithm>
#include "CpuSimpleFlatCacheStatic.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
/**
 * Default constructor
**/
template <int waySize,int associativity>
CpuSimpleFlatCacheStatic<waySize,associativity>::CpuSimpleFlatCacheStatic(void)
{
	//size
	const int size = waySize *  SIMPLE_FLAT_CACHE_LINE_SIZE * associativity;

	//checks
	assert(size > 0);
	assert(associativity < size / SIMPLE_FLAT_CACHE_LINE_SIZE);
	assert(size % associativity == 0);
	assert((size / associativity) %  SIMPLE_FLAT_CACHE_LINE_SIZE == 0);
	
	//init
	for (size_t row = 0 ; row < waySize ; row++) {
		this->tagCurrentAge[row] = associativity;
		for (size_t way = 0 ; way < associativity ; way++) {
			this->tagLineVirtIndex[row][way] = 0;
			this->tagLineAge[row][way] = way;
		}
	}
}

/*******************  FUNCTION  *********************/
/**
 * Destructor to clear memory.
**/
template <int waySize,int associativity>
CpuSimpleFlatCacheStatic<waySize,associativity>::~CpuSimpleFlatCacheStatic(void)
{
}

/*******************  FUNCTION  *********************/
template <int waySize,int associativity>
bool CpuSimpleFlatCacheStatic<waySize,associativity>::onMemoryAccessInlined(size_t addr)
{
	//row
	size_t addrIndex = getVirtIndex(addr);
	size_t row = getRow(addrIndex);

	//check if is in cache
	int hitWay = -1;
	for (size_t way = 0 ; way < associativity; way++) {
		//if index match => hit
		if (this->tagLineVirtIndex[row][way] == addrIndex) {
			hitWay = way;
			break;
		}
	}

	//if hit, update time otherwise evict one
	if (hitWay != -1) {
		this->tagLineAge[row][hitWay] = this->tagCurrentAge[row];
	} else {
		size_t victim = this->selectVictim(row);
		this->tagLineAge[row][victim] = this->tagCurrentAge[row];
		//printf("Evict row=%lu, way=%lu => oldAddrIndex=%lx, addrIndex=%lx\n",row,victim,this->tagLineVirtIndex[base + victim],addrIndex);
		this->tagLineVirtIndex[row][victim] = addrIndex;
	}

	//update age
	this->tagCurrentAge[row]++;

	//if age reach limit, need to scale down everything
	if (this->tagCurrentAge[row] == 0) {
		this->scaleDownAges(row);
		//we set gap to always be larger to scaled down values
		this->tagCurrentAge[row] = associativity;
		//printf("Next age = %lu\n",associativity);
	}

	//retu
	return (hitWay != -1);
}

/*******************  FUNCTION  *********************/
template <int waySize,int associativity>
bool CpuSimpleFlatCacheStatic<waySize,associativity>::onMemoryAccess(size_t addr)
{
	return onMemoryAccessInlined(addr);
}

/*******************  FUNCTION  *********************/
/**
 * We use a uint8_t age counter, so when we reach limit we need to scale down all
 * the values.
 * This is a trick so we do not have to update age counter for every step only once
 * every 256 address to the cache row.
**/
template <int waySize,int associativity>
void CpuSimpleFlatCacheStatic<waySize,associativity>::scaleDownAges(size_t row) 
{
	//build array
	for (size_t way = 0 ; way < associativity ; way++)
		this->bufSortAges[way] = this->tagLineAge[row][way];

	//sort
	std::sort(this->bufSortAges,this->bufSortAges+associativity);

	//apply
	for (size_t newAge = 0 ; newAge < associativity ; newAge++) {
		SimpleFlatCacheLRUAge oldAge = this->bufSortAges[newAge];
		for (size_t way = 0 ; way < associativity ; way++) {
			if (this->tagLineAge[row][way] == oldAge) {
				//printf("Scale down row=%lu, way=%d, oldAge=%lu, newAge=%lu\n",row,way,(size_t)oldAge,(size_t)newAge);
				this->tagLineAge[row][way] = newAge;
				break;
			}
		}
	}
}

/*******************  FUNCTION  *********************/
/**
 * This function select the victim cache entry to evict.
**/
template <int waySize,int associativity>
size_t CpuSimpleFlatCacheStatic<waySize,associativity>::selectVictim(size_t row) const
{
	//start by selecting first way
	size_t index = 0;
	size_t minAge = this->tagLineAge[row][index];

	//search one with lowest age
	for (size_t way = 1 ; way < associativity ; way++) {
		SimpleFlatCacheLRUAge age = this->tagLineAge[row][way];
		if (age < minAge) {
			minAge = age;
			index = way;
		}
	}

	//ret
	return index;
}

/*******************  FUNCTION  *********************/
template <int waySize,int associativity>
void CpuSimpleFlatCacheStatic<waySize,associativity>::onThreadMove(const CpuBindList & cpuList)
{
	//nothing to do
}

}

#endif //CPU_SIMPLE_FLAT_CACHE_STATIC_IMPL_HPP
