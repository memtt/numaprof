
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
#include "../common/StaticAssoCache.hpp"
#include "../../../extern-deps/from-fftw/cycle.h"

/********************  MACRO  ***********************/
#define REPEAT 400
#define INSTR 512

/***************** USING NAMESPACE ******************/
using namespace numaprof;

/*********************  TYPES  **********************/
typedef StaticAssoCache<size_t,4,32> TLB;

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	//set map
	std::map<size_t,size_t> instr;
	for (int i = 0 ; i < INSTR ; i++)
		instr[i] = i;
	
	//cache
	TLB cache;
	
	//measure page table
	ticks t0 = getticks();
	for (int i = 0 ; i < INSTR ; i++)
		for (int r = 0 ; r < REPEAT ; r++)
			instr[i];
	ticks t1 = getticks();
	printf("Map   : %llu\n",t1-t0);
	
	//measure with cache
	t0 = getticks();
	size_t ref;
	for (int i = 0 ; i < INSTR ; i++)
	{
		for (int r = 0 ; r < REPEAT ; r++)
		{
			size_t * page = cache.get(i);
			if (page == NULL)
				cache.set(i,&ref);
		}
	}
	t1 = getticks();
	printf("Cache : %llu\n",t1-t0);
		
	return EXIT_SUCCESS;
}
