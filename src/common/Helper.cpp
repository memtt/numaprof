/*****************************************************
             PROJECT  : numaprof
             VERSION  : 0.0.0-dev
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

/*******************  FUNCTION  ********************/
bool Helper::contain(const char * in, const char * what)
{
	return strstr(in,what) != NULL;
}

}
