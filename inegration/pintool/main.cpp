#include <thread>
#include <functional>
#include <cstdio>

int func()
{
	char buffer[128];
	sprintf(buffer,"Hellow world!");
	printf("%s\n",buffer);
}

int main()
{
	printf("ok\n");
	std::thread a(func);
	sleep(1);
	std::thread b(func);
	a.join();
	b.join();
}
