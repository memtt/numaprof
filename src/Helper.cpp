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
#include "Debug.hpp"
#include "Helper.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/********************  GLOBALS  *********************/
static const char * cstExeFile = "/proc/self/exe";
static const char * cstCmdFile = "/proc/self/cmdline";

/*******************  FUNCTION  *********************/
char * Helper::loadTxtFile(const char * path,size_t maxSize)
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
bool Helper::extractNth(char * out,const char * value,char sep,int index)
{
	//search start
	const char * start = value;
	while (index > 0 && *start != '\0')
	{
		if (*start == sep)
			index--;
		start++;
	}

	//check not found
	if (*start == '\0')
		return false;

	//copy until end
	while (*start != '\0' && *start != sep)
	{
		*out = *start;
		out++;
		start++;
	}

	//close
	*out = '\0';
	return true;
}

/*******************  FUNCTION  ********************/
std::string Helper::getExeName(void)
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
std::string Helper::getCmdLine(void)
{
	return loadTxtFile(cstCmdFile);
}

/*******************  FUNCTION  *********************/
std::string Helper::getHostname(void)
{
	char buffer[4096];
	int res = gethostname(buffer,sizeof(buffer));
	numaprofAssume(res != 0,"Failed to read hostname with getHostname() !");
	return buffer;
}

/*******************  FUNCTION  *********************/
std::string Helper::getDateTime(void)
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

/*******************  FUNCTION  ********************/
//extect format "0-39"
Range::Range(const char * value)
{
	//first value
	start = atoi(value);

	//move to second, search pos of '-'
	const char * second = value;
	while(*second != '-' && *second != '\0')
		second++;

	//parse second
	if (*second == '\0')
		end = start;
	else
		end = atoi(second+1);
}

/*******************  FUNCTION  ********************/
bool Range::contain(int value)
{
	return value >= start && value <= end;
}

/*******************  FUNCTION  ********************/
/**
 * Check if a given string end by a reference.
 * @param value String to check
 * @param what Need to end by this.
**/
bool Helper::endBy(const std::string & value,const std::string & what)
{
	//if too large
	if (what.size() > value.size())
		return false;
	
	//check
	return (strncmp(value.c_str()+value.size()-what.size(),what.c_str(),what.size()) == 0);
}

}
