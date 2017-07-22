/*****************************************************
			 PROJECT  : numaprof
			 VERSION  : 2.3.0
			 DATE     : 05/2017
			 AUTHOR   : Valat Sébastien - CERN
			 LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <cassert>
#include "Debug.hpp"
#include "Helper.hpp"
#include "NumaTopo.hpp"

//we cannot use the standard sched_get_affinity function in pin :(
#ifdef USE_PIN_LOCKS
	#include <sys/syscall.h> 
	#if !defined(__NR_sched_getaffinity)
		#if defined(__x86_64__)
			#define __NR_sched_getaffinity 204
		#else
			#error "Unsupported arch !"
		#endif
	#endif
	#define numactl_sched_getaffinity(pid,nb,mask) syscall(__NR_sched_getaffinity,(pid),(nb),(mask))
#else
	#define numactl_sched_getaffinity sched_getaffinity
#endif

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

/*******************  FUNCTION  *********************/
int NumaTopo::getCurrentNumaAffinity(void)
{
	//check
	assert(cpus <= CPU_SETSIZE);
	//printf("%d <= %d\n",cpus,CPU_SETSIZE);

	//get from system
	cpu_set_t mask;
	long status = numactl_sched_getaffinity(0,cpus,&mask);
	if (status < 0)
	{
		printf("status = %ld\n",status);
		perror("Invalid syscall to sched_getaffinity");
		return -1;
	}

	//map to numa
	return getCurrentNumaAffinity(mask);
}

/*******************  FUNCTION  *********************/
int NumaTopo::getCurrentNumaAffinity(cpu_set_t & mask)
{
	int numa = -1;

	for (int i = 0 ; i < cpus ; i++)
	{
		if (CPU_ISSET(i,&mask))
		{
			printf("Thread can be on %d, numa = %d\n",i,numaMap[i]);
			if (numa == -1)
				numa = numaMap[i];
			if (numa != numaMap[i])
				return -1;
		}
	}

	return numa;
}

}
