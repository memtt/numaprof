/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include <cstdlib>
#include <cstdio>
#include "../core/PageTable.hpp"
#include "../core/MallocTracker.hpp"

/***************** USING NAMESPACE ******************/
using namespace numaprof;

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	//check
	if (argc != 2)
	{
		printf("Usage : %s {INPUT_FILE}\n",argv[0]);
		return EXIT_FAILURE;
	}
	
	//load input 
	FILE * fp = fopen(argv[1],"r");
	if (fp == NULL)
	{
		perror(argv[1]);
		return EXIT_FAILURE;
	}
	
	//build page table & alloc tracker
	PageTable table;
	MallocTracker tracker(&table);
	
	//loopo
	while (!feof(fp))
	{
		char buffer[1024];
		char * ret = fgets(buffer,sizeof(buffer),fp);
		if (ret != NULL)
		{
			size_t size;
			StackIp ip = 0;
			void * ptr;
			if (sscanf(buffer,"TRACE: tracker.onAlloc(ip,%p,%lu)",&ptr,&size) == 2)
				tracker.onAlloc(ip,(size_t)ptr,size);
			else if (sscanf(buffer,"TRACE: tracker.onFree(%p)",&ptr) == 1)
				tracker.onFree((size_t)ptr);
		}
	}
	
	//finish
	fclose(fp);
	return EXIT_SUCCESS;
}
