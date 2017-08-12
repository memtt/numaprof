/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <cstdio>
#include <cstdlib>

/********************  GLOBALS  *********************/
#define SIZE (200*1024*1024/sizeof(float))

/*******************  FUNCTION  *********************/
void badFirstAccess()
{
	float * buffer = new float[SIZE];

	//do first touch from main thread
	for (size_t i = 0 ; i < SIZE ; i++)
		buffer[i] = i;
	
	//now do access in threads
	#pragma omp parallel for
	for (size_t i = 0 ; i < SIZE ; i++)
		buffer[i]++;
	
	delete [] buffer;
}

/*******************  FUNCTION  *********************/
void betterFirstAccess()
{
	float * buffer = new float[SIZE];

	//do first touch from all threads in same way than access
	#pragma omp parallel for
	for (size_t i = 0 ; i < SIZE ; i++)
		buffer[i] = i;
	
	//now do access in threads
	#pragma omp parallel for
	for (size_t i = 0 ; i < SIZE ; i++)
		buffer[i]++;
	
	delete [] buffer;
}

/*******************  FUNCTION  *********************/
int main()
{
	printf("Running bad first touch...\n");
	badFirstAccess();
	
	printf("Running better first touch...\n");
	betterFirstAccess();
	
	return EXIT_SUCCESS;
}
