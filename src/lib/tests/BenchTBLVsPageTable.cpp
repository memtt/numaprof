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
#include "../common/StaticAssoCache.hpp"
#include "../core/PageTable.hpp"
#include "../../../extern-deps/from-fftw/cycle.h"

/********************  MACRO  ***********************/
#define REPEAT 400
#define SIZE 1024*1024

/***************** USING NAMESPACE ******************/
using namespace numaprof;

/*********************  TYPES  **********************/
typedef StaticAssoCache<Page,4,32> TLB;

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	//buf
	int * buffer = new int[SIZE];
	memset(buffer,0,SIZE*sizeof(int));
	
	//table & tlb
	TLB tlb;
	PageTable table;
	
	//first set page
	for (int i = 0 ; i < SIZE ; i++)
		table.getPage((size_t)&buffer[i]);
	
	//measure page table
	ticks t0 = getticks();
	for (int i = 0 ; i < SIZE ; i++)
		for (int r = 0 ; r < REPEAT ; r++)
			table.getPage((size_t)&buffer[i]);
	ticks t1 = getticks();
	printf("Page table : %llu\n",t1-t0);
	
	//measure with cache
	t0 = getticks();
	Page p;
	for (int i = 0 ; i < SIZE ; i++)
	{
		for (int r = 0 ; r < REPEAT ; r++)
		{
			Page * page = tlb.get((size_t)&buffer[i]>>NUMAPROF_PAGE_OFFSET);
			if (page == NULL)
				tlb.set((size_t)&buffer[i]>>NUMAPROF_PAGE_OFFSET,&p);
		}
	}
	t1 = getticks();
	printf("TLB        : %llu\n",t1-t0);
		
	return EXIT_SUCCESS;
}
