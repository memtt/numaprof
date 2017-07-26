#include <thread>
#include <functional>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sched.h>

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

char * global;

int func2(int id)
{
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(id,&set);
	long status = sched_setaffinity(0,CPU_SETSIZE,&set);
	printf("status : %ld\n",status);

	char * buffer = (char*)malloc(128);
	sprintf(buffer,"Hellow world!");
	printf("%s\n",buffer);
	free(buffer);

	for (int i = 0 ; i < 1 ; i++)
	{
		buffer = new char[20*1024*1024];
		for (int i = 0 ; i < 20*1024*1024 ; i++)
			buffer[i] = global[i];
		delete [] buffer;
	}
}

int main(int argc, char ** argv)
{
	int ncpu = 1;
	if (argc == 2)
		ncpu = atoi(argv[1]);
	printf("ok\n");
	std::thread a(func);
	a.join();
	std::thread lst[40];
	global = new char[20*1024*1024];
	for (int i = 0 ; i < 20*1024*1024 ; i++)
		global[i] = 1;
	for (int i = 0 ; i < ncpu ; i++)
		lst[i] = std::thread([i](){func2(i);});
	for (int i = 0 ; i < ncpu ; i++)
		lst[i].join();
}
