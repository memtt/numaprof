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
#include <unistd.h>

/*******************  FUNCTION  *********************/
int func()
{
	char * buffer = (char*)malloc(128);
	sprintf(buffer,"Hellow world!");
	printf("%s\n",buffer);
	free(buffer);

	buffer = new char[20*1024*1024];
	for (int i = 0 ; i < 20*1024*1024 ; i++)
		buffer[i] = 0;
	delete [] buffer;
}

/*******************  FUNCTION  *********************/
int main()
{
	printf("ok\n");
	func();
	func();
}
