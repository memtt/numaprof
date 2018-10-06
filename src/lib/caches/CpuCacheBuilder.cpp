/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.0-dev
             DATE     : 02/2018
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include <cstdio>
#include "CpuCacheBuilder.hpp"
#include "CpuCacheDummy.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
/**
 * Build the global cache layout if we want a complex topology. This object
 * will be shared by all cache object inside thread and can represent a
 * shared topology to handle shared caches.
 * @param type Define the type of cache to build.
**/
void * CpuCacheBuilder::buildLayout(const std::string & type)
{
	if (type == "dummy") {
		return NULL;
	} else {
		fprintf(stderr,"NUMARPOF: Invalid cache type, cannot build cache layout: '%s'\n",type.c_str());
		exit(1);
	}
}

/*******************  FUNCTION  *********************/
/**
 * Function used to destroy layout at the end of execution.
**/
void CpuCacheBuilder::destroyLayout(const std::string & type,void * layout)
{
	if (type == "dummy") {
		//nothing to do
	} else {
		fprintf(stderr,"NUMARPOF: Invalid cache type, cannot destroy cache layout: '%s'\n",type.c_str());
		exit(1);
	}
}

/*******************  FUNCTION  *********************/
/**
 * Main function to build cache based on parameters.
 * @param type Define the type of cache to build.
**/
CpuCache * CpuCacheBuilder::buildCache(const std::string & type,void * layout)
{
	if (type == "dummy") {
		return buildDummyCache();
	} else {
		fprintf(stderr,"NUMARPOF: Invalid cache type, cannot build : '%s'\n",type.c_str());
		exit(1);
	}
}

/*******************  FUNCTION  *********************/
/**
 * Build dummy cache.
**/
CpuCache * CpuCacheBuilder::buildDummyCache(void)
{
	return new CpuCacheDummy();
}

}
