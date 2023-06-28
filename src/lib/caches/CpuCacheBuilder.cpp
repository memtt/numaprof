/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include <cstdio>
#include "../common/Debug.hpp"
#include "CpuCacheBuilder.hpp"
#include "CpuCacheDummy.hpp"
#include "CpuSimpleFlatCache.hpp"
#include "CpuSimpleFlatCacheStatic.hpp"
#include "../common/Options.hpp"

/*******************  CONSTS  ***********************/
#define BUILDER_MODE_DUMMY "dummy"
#define BUILDER_MODE_SIMPLE_FLAT "L1"
#define BUILDER_MODE_SIMPLE_FLAT_STATIC "L1_static"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  TYPES  **********************/
typedef CpuSimpleFlatCacheStatic<64,8> CpuStaticL1Cache;

/*******************  FUNCTION  *********************/
/**
 * Build the global cache layout if we want a complex topology. This object
 * will be shared by all cache object inside thread and can represent a
 * shared topology to handle shared caches.
 * @param type Define the type of cache to build.
**/
void * CpuCacheBuilder::buildLayout(const std::string & type)
{
	if (type == BUILDER_MODE_DUMMY) {
		return NULL;
	} else if (type == BUILDER_MODE_SIMPLE_FLAT) {
		return NULL;
	} else if (type == BUILDER_MODE_SIMPLE_FLAT_STATIC) {
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
	if (type == BUILDER_MODE_DUMMY) {
		//nothing to do
	} else if (type == BUILDER_MODE_SIMPLE_FLAT) {
		//nothing to do
	} else if (type == BUILDER_MODE_SIMPLE_FLAT_STATIC) {
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
	if (type == BUILDER_MODE_DUMMY) {
		return buildDummyCache();
	} else if (type == BUILDER_MODE_SIMPLE_FLAT) {
		return buildSimpleFlatCache();
	} else if (type == BUILDER_MODE_SIMPLE_FLAT_STATIC) {
		return buildSimpleFlatCacheStatic();
	} else {
		fprintf(stderr,"NUMARPOF: Invalid cache type, cannot build : '%s'\n",type.c_str());
		exit(1);
	}
}

/*******************  FUNCTION  *********************/
/**
 * Build a static L1 cache which might go faster than configurable
 * one.
**/
CpuCache * CpuCacheBuilder::buildSimpleFlatCacheStatic(void)
{
	return new CpuStaticL1Cache();
}

/*******************  FUNCTION  *********************/
/**
 * Build dummy cache.
**/
CpuCache * CpuCacheBuilder::buildDummyCache(void)
{
	return new CpuCacheDummy();
}

/*******************  FUNCTION  *********************/
/**
 * Build dummy cache.
**/
CpuCache * CpuCacheBuilder::buildSimpleFlatCache(void)
{
	const Options & options = getGlobalOptions();
	size_t cacheSize = convertHumanUnit(options.cacheSize);
	return new CpuSimpleFlatCache(cacheSize,options.cacheAssociativity);
}

/*******************  FUNCTION  *********************/
/**
 * Extract value from given string human unit.
 * Supported is K, M, G, T with multiple of 1024 (not 1000 !).
**/
size_t CpuCacheBuilder::convertHumanUnit(const std::string & value)
{
	size_t size;
	if (sscanf(value.c_str(),"%luK",&size) == 1) {
		return size * 1024UL;
	} else if (sscanf(value.c_str(),"%luM",&size) == 1) {
		return size * 1024UL * 1024UL;
	} else if (sscanf(value.c_str(),"%luG",&size) == 1) {
		return size * 1024UL * 1024UL * 1024UL;
	} else if (sscanf(value.c_str(),"%luT",&size) == 1) {
		return size * 1024UL * 1024UL * 1024UL * 1024UL;
	} else if (sscanf(value.c_str(),"%lu",&size) == 1) {
		return size;
	} else {
		numaprofFatal("Invalid format for cache:size in config : %s, should be {XXX}[K|M|G|T]\n",value.c_str());
		return 0;
	}
}

}
