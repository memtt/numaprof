
/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <map>
#include "../caches/CpuCacheDummy.hpp"
#include "../caches/CpuSimpleFlatCache.hpp"
#include "../caches/CpuSimpleFlatCacheStatic.hpp"
#include "../../../extern-deps/from-fftw/cycle.h"

/********************  MACRO  ***********************/
#define REPEAT 10000
#define INSTR 512

/***************** USING NAMESPACE ******************/
using namespace numaprof;

/*********************  TYPES  **********************/
typedef CpuSimpleFlatCacheStatic<64,8> L1StaticCache;

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	//set map
	const size_t size = 32*1024;
	char buffer[size];
	
	//cache
	L1StaticCache staticCache;
	CpuSimpleFlatCache dynCache(size,8);
	CpuCacheDummy dummyCache;
	
	//no cache access
	ticks t0 = getticks();
	for (int r = 0 ; r < REPEAT ; r++)
		for (int i = 0 ; i < size ; i++)
			buffer[i] = 0;
	ticks t1 = getticks();
	printf("No cache             : %llu\n",t1-t0);
	
	//measure with cache
	t0 = getticks();
	for (int r = 0 ; r < REPEAT ; r++) {
		for (int i = 0 ; i < size ; i++) {
			buffer[i] = 0;
			dummyCache.onMemoryAccess((size_t)buffer+i);
		}
	}
	t1 = getticks();
	printf("Dummy cache          : %llu\n",t1-t0);

	//measure with cache
	t0 = getticks();
	for (int r = 0 ; r < REPEAT ; r++) {
		for (int i = 0 ; i < size ; i++) {
			buffer[i] = 0;
			staticCache.onMemoryAccessInlined((size_t)buffer+i);
		}
	}
	t1 = getticks();
	printf("Static cache inlined : %llu\n",t1-t0);

	//measure with cache
	t0 = getticks();
	for (int r = 0 ; r < REPEAT ; r++) {
		for (int i = 0 ; i < size ; i++) {
			buffer[i] = 0;
			staticCache.onMemoryAccess((size_t)buffer+i);
		}
	}
	t1 = getticks();
	printf("Static cache         : %llu\n",t1-t0);

	//measure with cache
	t0 = getticks();
	for (int r = 0 ; r < REPEAT ; r++) {
		for (int i = 0 ; i < size ; i++) {
			buffer[i] = 0;
			dynCache.onMemoryAccess((size_t)buffer+i);
		}
	}
	t1 = getticks();
	printf("Dynamic cache        : %llu\n",t1-t0);

	return EXIT_SUCCESS;
}
