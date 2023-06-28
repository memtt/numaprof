/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/




























































































/********************  HEADERS  *********************/
#include <cstdio>
#include <cstdlib>
#include <omp.h>

#define SIZE_PRE_THREAD (200*1024*1024/sizeof(float))

/*******************  FUNCTION  *********************/
void badFirstAccess(size_t size)
{
	float * buffer = new float[size];

	//do first touch from main thread
	for (size_t i = 0 ; i < size ; i++)
		buffer[i] = i;
	
	//now do access in threads
	#pragma omp parallel for
	for (size_t i = 0 ; i < size ; i++)
		buffer[i]++;
	
	delete [] buffer;
}

/*******************  FUNCTION  *********************/
void betterFirstAccess(size_t size)
{
	float * buffer = new float[size];

	//do first touch from all threads in same way than access
	#pragma omp parallel for
	for (size_t i = 0 ; i < size ; i++)
		buffer[i] = i;
	
	//now do access in threads
	#pragma omp parallel for
	for (size_t i = 0 ; i < size ; i++)
		buffer[i]++;
	
	delete [] buffer;
}

/*******************  FUNCTION  *********************/
int main()
{
	size_t threads = omp_get_num_threads();
	size_t size = threads * SIZE_PRE_THREAD;
	printf("Run on %lu threads\n",threads);
	
	printf("Running bad first touch...\n");
	badFirstAccess(size);
	
	printf("Running better first touch...\n");
	betterFirstAccess(size);
	
	return EXIT_SUCCESS;
}
