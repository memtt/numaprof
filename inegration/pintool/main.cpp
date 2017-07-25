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

int func2()
{
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(0,&set);
	long status = sched_setaffinity(0,CPU_SETSIZE,&set);
	printf("status : %ld\n",status);

	char * buffer = (char*)malloc(128);
	sprintf(buffer,"Hellow world!");
	printf("%s\n",buffer);
	free(buffer);

	buffer = new char[20*1024*1024];
	for (int i = 0 ; i < 20*1024*1024 ; i++)
		buffer[i] = 0;
	delete [] buffer;
}

int main()
{
	printf("ok\n");
	std::thread a(func);
	std::thread b(func2);
	a.join();
	b.join();
}
