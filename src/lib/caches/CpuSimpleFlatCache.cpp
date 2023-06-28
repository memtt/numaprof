/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include <cassert>
#include <algorithm>
#include "CpuSimpleFlatCache.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
/**
 * Default constructor
**/
CpuSimpleFlatCache::CpuSimpleFlatCache(size_t size,size_t associativity)
{
	//checks
	assert(size > 0);
	assert(associativity < size / SIMPLE_FLAT_CACHE_LINE_SIZE);
	assert(size % associativity == 0);
	assert((size / associativity) %  SIMPLE_FLAT_CACHE_LINE_SIZE == 0);

	//keep args
	this->size = size;
	this->associativity = associativity;
	this->waySize = (size / associativity) / SIMPLE_FLAT_CACHE_LINE_SIZE;

	//allocate
	this->tagLineVirtIndex = new size_t[associativity * this->waySize];
	this->tagLineAge = new SimpleFlatCacheLRUAge[associativity * this->waySize];
	this->tagCurrentAge = new SimpleFlatCacheLRUAge[this->waySize];
	this->bufSortAges = new SimpleFlatCacheLRUAge[associativity];

	//init
	for (size_t row = 0 ; row < waySize ; row++) {
		this->tagCurrentAge[row] = associativity;
		for (size_t way = 0 ; way < associativity ; way++) {
			this->tagLineVirtIndex[getIndex(row,way)] = 0;
			this->tagLineAge[getIndex(row,way)] = way;
		}
	}
}

/*******************  FUNCTION  *********************/
/**
 * Destructor to clear memory.
**/
CpuSimpleFlatCache::~CpuSimpleFlatCache(void)
{
	//free
	delete [] this->tagLineVirtIndex;
	delete [] this->tagLineAge;
	delete [] this->tagCurrentAge;
}

/*******************  FUNCTION  *********************/
bool CpuSimpleFlatCache::onMemoryAccess(size_t addr)
{
	//row
	size_t addrIndex = getVirtIndex(addr);
	size_t row = getRow(addrIndex);
	size_t base = row * associativity;

	//check if is in cache
	int hitWay = -1;
	for (size_t way = 0 ; way < this->associativity; way++) {
		//if index match => hit
		if (this->tagLineVirtIndex[base + way] == addrIndex) {
			hitWay = way;
			break;
		}
	}

	//if hit, update time otherwise evict one
	if (hitWay != -1) {
		this->tagLineAge[base + hitWay] = this->tagCurrentAge[row];
	} else {
		size_t victim = this->selectVictim(row);
		this->tagLineAge[base + victim] = this->tagCurrentAge[row];
		//printf("Evict row=%lu, way=%lu => oldAddrIndex=%lx, addrIndex=%lx\n",row,victim,this->tagLineVirtIndex[base + victim],addrIndex);
		this->tagLineVirtIndex[base + victim] = addrIndex;
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
/**
 * We use a uint8_t age counter, so when we reach limit we need to scale down all
 * the values.
 * This is a trick so we do not have to update age counter for every step only once
 * every 256 address to the cache row.
**/
void CpuSimpleFlatCache::scaleDownAges(size_t row) 
{
	//vars
	size_t base = row * associativity;

	//build array
	for (size_t way = 0 ; way < associativity ; way++)
		this->bufSortAges[way] = this->tagLineAge[base + way];

	//sort
	std::sort(this->bufSortAges,this->bufSortAges+associativity);

	//apply
	for (size_t newAge = 0 ; newAge < this->associativity ; newAge++) {
		SimpleFlatCacheLRUAge oldAge = this->bufSortAges[newAge];
		for (size_t way = 0 ; way < this->associativity ; way++) {
			if (this->tagLineAge[base + way] == oldAge) {
				//printf("Scale down row=%lu, way=%d, oldAge=%lu, newAge=%lu\n",row,way,(size_t)oldAge,(size_t)newAge);
				this->tagLineAge[base + way] = newAge;
				break;
			}
		}
	}
}

/*******************  FUNCTION  *********************/
/**
 * This function select the victim cache entry to evict.
**/
size_t CpuSimpleFlatCache::selectVictim(size_t row) const
{
	//start by selecting first way
	size_t base = row * associativity;
	size_t minAge = this->tagLineAge[base];
	size_t index = 0;

	//search one with lowest age
	for (size_t way = 1 ; way < this->associativity ; way++) {
		SimpleFlatCacheLRUAge age = this->tagLineAge[base+way];
		if (age < minAge) {
			minAge = age;
			index = way;
		}
	}

	//ret
	return index;
}

/*******************  FUNCTION  *********************/
void CpuSimpleFlatCache::onThreadMove(const CpuBindList & cpuList)
{
	//nothing to do
}

}
