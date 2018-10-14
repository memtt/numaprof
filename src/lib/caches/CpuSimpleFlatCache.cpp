/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.0-dev
             DATE     : 02/2018
             AUTHOR   : Valat Sébastien
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include <cassert>
#include "CpuSimpleFlatCache.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
/**
 * Default constructor
**/
CpuSimpleFlatCache::CpuSimpleFlatCache(size_t size,size_t cacheLineSize,size_t associativity)
{
	//checks
	assert(size > 0);
	assert(associativity < size / cacheLineSize);
	assert(size % associativity == 0);
	assert((size / associativity) %  cacheLineSize == 0);

	//keep args
	this->size = size;
	this->cacheLineSize = cacheLineSize;
	this->associativity = associativity;
	this->waySize = (size / associativity) / cacheLineSize;
}

/*******************  FUNCTION  *********************/
bool CpuSimpleFlatCache::onMemoryAccess(size_t addr)
{
	return false;
}

/*******************  FUNCTION  *********************/
void CpuSimpleFlatCache::onThreadMove(const CpuBindList & cpuList)
{
	//nothing to do
}

}
