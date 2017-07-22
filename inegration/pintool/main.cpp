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
}

int main()
{
	printf("ok\n");
	std::thread a(func);
	sleep(1);
	std::thread b(func2);
	a.join();
	b.join();
}
