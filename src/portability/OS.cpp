/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include "../common/Debug.hpp"
#include "OS.hpp"
#include <sys/types.h>
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

}
