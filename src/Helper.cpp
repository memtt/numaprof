/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include <cstdio>
#include "Helper.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

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

}
