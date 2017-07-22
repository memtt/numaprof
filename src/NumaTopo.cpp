/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include "Debug.hpp"
#include "Helper.hpp"
#include "NumaTopo.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
NumaTopo::NumaTopo(void)
{
    this->cpus = 0;
    this->numaNodes = 0;
    this->numaMap = NULL;

    this->loadCpuNb();
    this->loadNumaMap();
}

/*******************  FUNCTION  *********************/
NumaTopo::~NumaTopo(void)
{
    delete [] numaMap;
}

/*******************  FUNCTION  *********************/
void NumaTopo::loadCpuNb()
{
    //load present
    //format is "0-39"
    char * present = Helper::loadTxtFile("/sys/devices/system/cpu/present",256);

    //scan all ranges
    char tmp[64];
    int i = 0;
    this->cpus = 0;
    while (Helper::extractNth(tmp,present,',',i++))
    {
        Range range(tmp);
        if (range.end > this->cpus)
            this->cpus = range.end;
    }

    //clear mem
    delete [] present;

    //inc
    this->cpus++;

    //debug
    printf("Get cpu number : %d\n",cpus);
}

/*******************  FUNCTION  *********************/
void NumaTopo::loadNumaMap(void)
{
    //allocate
    numaMap = new int[cpus];
    for (int i = 0 ; i < cpus ; i++)
        numaMap[i] = -1;

    //loop on all numa nodes (until we do not found one)
    int node = 0;
    while (true)
    {
        //build file path
        char fname[128];
        sprintf(fname,"/sys/devices/system/node/node%d/cpulist",node);

        //load
        char * list = Helper::loadTxtFile(fname,256);
        if (list == NULL)
            break;
        
        //scan all ranges
        char tmp[64];
        int i = 0;
        this->cpus = 0;
        while (Helper::extractNth(tmp,list,',',i++))
        {
            Range range(tmp);
            for (int j = 0 ; j < cpus ; j++)
                if (range.contain(j))
                    numaMap[j] = node;
        }

        //clear
        delete [] list;
    }

    //check and warn
    for (int i = 0 ; i < cpus ; i++)
    {
        if (numaMap[i] == -1)
        {
            printf("Caution, CPU %d was not assigned to a NUMA node, assigned to 0 !\n",i);
            numaMap[i] = 0;
        }
    }
}

}
