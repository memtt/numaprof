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
#include <cstring>

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	int repeat = 100;
	if (argc == 1)
		repeat = atoi(argv[1]);
	size_t size = 1024*1024*1024;
	size_t elmtSize = 128;
	size_t elmtNb = size/ elmtSize;
	for (int i = 0 ; i < repeat ; i++)
	{
		printf("Loop alloc%d\n",i);
		char ** array = new char*[elmtNb];
		for (size_t j = 0 ; j < elmtNb ; j++)
			array[j] = new char[elmtSize];
		//printf("Loop set%d\n",i);
		//for (size_t j = 0 ; j < elmtNb ; j++)
		//	memset(array[j],0,elmtSize);
		printf("Loop free%d\n",i);
		for (size_t j = 0 ; j < elmtNb ; j++)
			delete [] array[j];
		delete [] array;
	}
	return EXIT_SUCCESS;
}
