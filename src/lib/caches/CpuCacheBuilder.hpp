/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.0-dev
             DATE     : 02/2018
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef CPU_CACHE_BUILDER_HPP
#define CPU_CACHE_BUILDER_HPP

/********************  HEADERS  *********************/
#include "CpuCache.hpp"

/********************  NAMESPACE  *******************/
namespace numaprof
{

/*********************  CLASS  **********************/
/**
 * Class used to build cache system depending on policy and layout.
**/
class CpuCacheBuilder
{
	public:
		static void * buildLayout(const std::string & type);
		static void destroyLayout(const std::string & type,void * layout);
		static CpuCache * buildCache(const std::string & type,void * layout);
	private:
		CpuCacheBuilder(void);
		static CpuCache * buildDummyCache(void);
};

}

#endif //CPU_CACHE_BUILDER_HPP
