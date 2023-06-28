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
#include "Debug.hpp"
#include "Helper.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  ********************/
/**
 * Extract the nth sub-string considering a separator.
 * @param out Buffer on which to write.
 * @param value Input string to split
 * @param sep Separator to consider.
 * @param index Index of the sub-string to extract.
 * @return Return true, if found the sub-string, false otherwise.
**/
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
/**
 * Build a range from a string format (eg. 10-35).
 * @param value Define the default value to setup. Should follow format (10-35).
**/
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
	
	//check
	numaprofAssert(end >= start);
}

/*******************  FUNCTION  ********************/
/**
 * Check if the range object contain the given value. The max value of range is considered
 * inside by contain function, meaning 10-20 return true on contain(20).
 * @param value Define the value we want to check.
**/
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
/**
 * Check if a given string contain another one.
 * @param in Define the input string.
 * @param what Degine which string to search in.
**/
bool Helper::contain(const char * in, const char * what)
{
	return strstr(in,what) != NULL;
}

/*******************  FUNCTION  *********************/
/**
 * Check if the string is an integer value.
*/
bool Helper::isInteger(const std::string & value)
{
	for (auto & it : value)
		if (it < '0' || it > '9')
			return false;
	return true;
}

}
