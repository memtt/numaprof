/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.0-dev
             DATE     : 02/2018
             AUTHOR   : Valat Sébastien
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef CPU_CACHE_DUMMY_HPP
#define CPU_CACHE_DUMMY_HPP

/********************  HEADERS  *********************/
#include "CpuCache.hpp"

/********************  NAMESPACE  *******************/
namespace numaprof
{

/*********************  CLASS  **********************/
/**
 * Provide a dummy cache which does not cache anything.
**/
class CpuCacheDummy : public CpuCache
{
	public:
		CpuCacheDummy(void);
		virtual bool onMemoryAccess(size_t addr);
		virtual void onThreadMove(const CpuBindList & cpuList);
};

}

#endif //CPU_CACHE_DUMMY_HPP
