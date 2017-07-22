/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef HELPER_HPP
#define HELPER_HPP

/*******************  HEADERS  **********************/
#include <cstdlib>

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  STRUCT  ***********************/
struct Range
{
    Range(const char * value);
    bool contain(int value);
    int start;
    int end;
};

/*******************  FUNCTION  *********************/
struct Helper
{
    static char * loadTxtFile(const char * path,size_t maxSize = 4096);
    static bool extractNth(char * out,const char * value,char sep,int index);
};

}

#endif //HELPER_HPP
