/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef CPU_CACHE_BUILDER_HPP
#define CPU_CACHE_BUILDER_HPP

/********************  HEADERS  *********************/
#include <string>
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
		static CpuCache * buildSimpleFlatCache(void);
		static CpuCache * buildSimpleFlatCacheStatic(void);
		static size_t convertHumanUnit(const std::string & value);
};

}

#endif //CPU_CACHE_BUILDER_HPP
