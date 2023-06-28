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
#include <unistd.h>

#define CNT 5

/*******************  FUNCTION  *********************/
void func()
{
	char * buffer = (char*)malloc(128);
	sprintf(buffer,"Hellow world!");
	printf("%s\n",buffer);
	free(buffer);

	char * lst[CNT];
	for (int i = 0 ; i < CNT ; i++)
		lst[i] = new char[20*1024*1024];
	for (int j = 0 ; j < CNT ; j++)
		for (int i = 0 ; i < 20*1024*1024 ; i++)
			lst[j][i] = 0;
	for (int i = 0 ; i < CNT ; i++)
		delete [] lst[i];
}

/*******************  FUNCTION  *********************/
int main()
{
	printf("ok\n");
	func();
	func();
	return EXIT_SUCCESS;
}
