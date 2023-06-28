/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef HELPER_HPP
#define HELPER_HPP

/*******************  HEADERS  **********************/
#include <cstdlib>
#include <string>

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  STRUCT  ***********************/
/** 
 * Provide range management.
**/
struct Range
{
	Range(const char * value);
	bool contain(int value);
	int start;
	int end;
};

/*******************  FUNCTION  *********************/
/**
 * Provide some helper function to be used in the code.
**/
struct Helper
{
	static bool extractNth(char * out,const char * value,char sep,int index);
	static bool endBy(const std::string & value,const std::string & what);
	static bool contain(const char * in, const char * what);
	static bool isInteger(const std::string & value);
};

}

#endif //HELPER_HPP
