/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <ctime>
#include "../common/Debug.hpp"
#include "OS.hpp"
#include <sys/types.h>
#ifdef __PIN__
	#include "../../extern-deps/from-numactl/MovePages.hpp"
#else
	#include "numaif.h"
#endif
#ifndef gettid
	#include <sys/syscall.h>
#endif

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/********************  MACROS  **********************/
//weird thing :(
#ifndef gettid
	#define gettid() syscall(__NR_gettid)
#endif
#define HUGE_PAGE_SIZE (2*1024*1024)
#define PAGE_SIZE 4096
#define HUGE_PAGE_SUB_PAGES (HUGE_PAGE_SIZE / PAGE_SIZE)

/********************  GLOBALS  *********************/
static const char * cstExeFile = "/proc/self/exe";
static const char * cstCmdFile = "/proc/self/cmdline";

/*******************  FUNCTION  *********************/
char * OS::loadTxtFile(const char * path,size_t maxSize)
{
	//open
	FILE * fp = fopen(path,"r");
	if (fp == NULL)
		return NULL;
	
	//allocate mem
	char * buffer = new char[maxSize];
	
	//load
	size_t size = fread(buffer,1,maxSize,fp);

	//close
	fclose(fp);

	//if not loaded
	if (size < 0)
	{
		delete [] buffer;
		return NULL;
	} else {
		buffer[size] = '\0';
	}

	//ret
	return buffer;
}

/*******************  FUNCTION  ********************/
std::string OS::getExeName(void)
{
	//buffer to read link
	char buffer[2048];
	
	//read
	size_t res = readlink(cstExeFile,buffer,sizeof(buffer));
	//assumeArg(res != (size_t)-1,"Fail to read link %1 : %2 !").arg(cstExeFile).argStrErrno().end();
	//assume(res < sizeof(buffer),"Fail to read link to get exe name. Maybe buffer is too small.");
	numaprofAssumePerror(res != (size_t)-1,"Fail to read link to get exe file !");
	numaprofAssume(res < sizeof(buffer),"Fail to read link to get exe name. Maybe buffer is too small.");
	
	
	//put \0
	buffer[res] = '\0';

	//extract exe from path
	char * name = basename(buffer);
	return name;
}

/*******************  FUNCTION  *********************/
std::string OS::getCmdLine(void)
{
	return loadTxtFile(cstCmdFile);
}

/*******************  FUNCTION  *********************/
std::string OS::getHostname(void)
{
	char buffer[4096];
	int res = gethostname(buffer,sizeof(buffer));
	numaprofAssume(res != 0,"Failed to read hostname with getHostname() !");
	return buffer;
}

/*******************  FUNCTION  *********************/
std::string OS::getDateTime(void)
{
	//vars
	char buffer[200];
	time_t t;
	struct tm *tmp;

	//get time
	t = time(NULL);
	tmp = localtime(&t);
	numaprofAssume(tmp != NULL,"Failed to get time with localtime() !");
	
	//convert to string format
	int res = strftime(buffer, sizeof(buffer), "%F %R", tmp);
	numaprofAssume(res > 0,"Failed to convert time to string format with strftime() !");
	
	return buffer;
}

/*******************  FUNCTION  *********************/
int OS::getPID(void)
{
	return getpid();
}

/*******************  FUNCTION  *********************/
int OS::getTID(void)
{
	return gettid();
}

/*******************  FUNCTION  *********************/
int OS::getNumaOfPage(size_t addr)
{
	static bool hasMovePages = true;

	//go fast
	if (hasMovePages == false)
		return 0;

	//4k align
	unsigned long page = (unsigned long)addr;
	page = page & (~(PAGE_SIZE-1));
	void * pages[1] = {(void*)page};
	int status;
	long ret = move_pages(0,1,pages,NULL,&status,0);
	if (ret == 0)
	{
		return status;
	} else {
		if (errno == ENOSYS)
		{
			fprintf(stderr,"\033[31mNUMAPROF: CAUTION, move_pages not implemented, you might be running on a non NUMA system !\nAll accesses will be considered local !\033[0m\n");
			hasMovePages = false;
		}
		return 0;
	}
}

/*******************  FUNCTION  *********************/
int OS::getNumaOfHugePage(size_t addr,bool * isHugePage)
{
	//trivial
	if (isHugePage == NULL)
		return getNumaOfPage(addr);
	
	//remember
	static bool hasMovePages = true;
	
	//default
	*isHugePage = true;

	//go fast
	if (hasMovePages == false)
		return 0;

	//2M align
	unsigned long page = (unsigned long)addr;
	
	//build storate
	page = page & (~(HUGE_PAGE_SIZE-1));
	void * pages[HUGE_PAGE_SUB_PAGES];
	for (int i = 0 ; i < HUGE_PAGE_SUB_PAGES ; i++)
		pages[i] = (void*)(page+i * PAGE_SIZE);
	
	//call
	int status[HUGE_PAGE_SUB_PAGES];
	long ret = move_pages(0,HUGE_PAGE_SUB_PAGES,pages,NULL,status,0);
	
	if (ret != 0)
	{
		if (errno == ENOSYS)
		{
			printf("\033[31mCAUTION, move_pages not implemented, you might be running on a non NUMA system !\nAll accesses will be considered local !\033[0m\n");
			hasMovePages = false;
		}
		return 0;
	}
	
	//check if huge page
	int first = status[0];
	for (int i = 0 ; i < HUGE_PAGE_SUB_PAGES ; i++)
	{
		if (status[i] != first)
		{
			*isHugePage = false;
			break;
		}
	}
	
	//interpret
	if (*isHugePage)
	{
		return first;
	} else {
		return status[(addr - page) / PAGE_SIZE];
	}
}

}
