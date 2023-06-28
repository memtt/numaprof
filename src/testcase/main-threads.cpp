/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <thread>
#include <functional>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sched.h>
#include <cstring>
#include <vector>

/********************  GLOBALS  *********************/
static char * global = NULL;

/*******************  FUNCTION  *********************/
void func()
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
void func2(int id)
{
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(id%256,&set);
	long status = sched_setaffinity(0,sizeof(cpu_set_t),&set);
	printf("status : %ld\n",status);

	char * buffer = (char*)malloc(128);
	sprintf(buffer,"Hellow world!");
	printf("%s\n",buffer);
	free(buffer);

	std::vector<int> vec;
	for (int i = 0 ; i < 10000 ; i++)
		vec.push_back(0);
	for (int i = 0 ; i < 10000 ; i++)
		vec[0]++;

	for (int i = 0 ; i < 1 ; i++)
	{
		buffer = new char[20*1024*1024];
		for (int i = 0 ; i < 20*1024*1024 ; i++)
			buffer[i] = global[i];
		delete [] buffer;
	}

	char * r = (char*)malloc(2*1024*1024);
	memset(r,0,2*1024*1024);
	r = (char*)realloc(r,4*1024*1024);
	memset(r,0,4*1024*1024);
	free(r);
}

/*******************  FUNCTION  *********************/
int main(int argc, char ** argv)
{
	int ncpu = 1;
	if (argc == 2)
		ncpu = atoi(argv[1]);
	printf("ok\n");
	global = new char[20*1024*1024];
	memset(global,0,20*1024*1024);
	std::thread a(func);
	a.join();
	std::thread lst[40];
	for (int i = 0 ; i < 20*1024*1024 ; i++)
		global[i] = 1;
	for (int i = 0 ; i < ncpu ; i++)
		lst[i] = std::thread([i](){func2(i);});
	for (int i = 0 ; i < ncpu ; i++)
		lst[i].join();
	
	return EXIT_SUCCESS;
}
