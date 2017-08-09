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
#include <cstdio>
#include "../common/Debug.hpp"
#include "../common/Helper.hpp"
#include "../portability/NumaTopo.hpp"
#include "../portability/OS.hpp"

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
	char * present = OS::loadTxtFile("/sys/devices/system/cpu/present",256);

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
	
	//allocate
	isMcdram = new bool[cpus];
	for (int i = 0 ; i < cpus ; i++)	
		isMcdram[i] = false;

	//loop on all numa nodes (until we do not found one)
	int node = 0;
	while (true)
	{
		//build file path
		char fname[128];
		sprintf(fname,"/sys/devices/system/node/node%d/cpulist",node);

		//load
		char * list = OS::loadTxtFile(fname,256);
		if (list == NULL)
			break;

		//replace \n
		char * it = list;
		while (*it != '\0')
		{
			if (*it == '\n')
				*it = '\0';
			else
				++it;
		}
		
		//scan all ranges
		char tmp[64];
		int i = 0;
		bool hasCpu = false;
		while (Helper::extractNth(tmp,list,',',i++))
		{
			Range range(tmp);
			for (int j = 0 ; j < cpus ; j++)
			{
				if (range.contain(j))
				{
					numaMap[j] = node;
					hasCpu = true;
				}
			}
		}

		//is mcdram
		if (hasCpu == false)
			isMcdram[node] = true;

		//clear
		delete [] list;

		//inc
		node++;
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
	
	//if no node
	if (node == 0)
		node = 1;
	
	//store
	numaNodes = node;
}

/*******************  FUNCTION  *********************/
int NumaTopo::getCurrentNumaAffinity(CpuBindList * cpuBindList)
{
	//check
	numaprofAssume(cpus <= CPU_SETSIZE,"To many CPU to be handled by cpu_set_t !");
	//printf("%d <= %d\n",cpus,CPU_SETSIZE);

	//get from system
	cpu_set_t mask;
	long status = numactl_sched_getaffinity(0,sizeof(cpu_set_t),&mask);
	if (status < 0)
	{
		printf("status = %ld\n",status);
		perror("Invalid syscall to sched_getaffinity");
		return -1;
	}

	//map to numa
	return getCurrentNumaAffinity(&mask, sizeof(cpu_set_t),cpuBindList);
}

/*******************  FUNCTION  *********************/
int NumaTopo::getCurrentNumaAffinity(cpu_set_t * mask, int size,CpuBindList * cpuBindList)
{
	int numa = -2;

	//clear
	if (cpuBindList != NULL)
		cpuBindList->clear();

	//check numa
	for (int i = 0 ; i < cpus ; i++)
	{
		if (CPU_ISSET(i,mask))
		{
			//printf("Thread can be on %d, numa = %d\n",i,numaMap[i]);
			if (numa == -2)
				numa = numaMap[i];
			else if (numa != numaMap[i])
				numa = -1;
			if (cpuBindList != NULL)
				cpuBindList->push_back(i);
		}
	}

	//might not append but in case...
	if (numa == -2)
		numa = -1;

	printf("Thread is binded on NUMA %d\n",numa);
	return numa;
}

/*******************  FUNCTION  *********************/
int NumaTopo::getNumaNodes ( void )
{
	return this->numaNodes;
}

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const NumaTopo& value)
{
	int nodes = value.numaNodes;
	if (nodes == 0)
		nodes = 1;
	
	json.openStruct();
		for (int node = 0 ; node < nodes ; node++)
		{
			char nodeName[128];
			sprintf(nodeName,"%d",node);
			json.openFieldStruct(nodeName);
				json.printField("isMcdram",value.isMcdram[node]);
				json.openFieldArray("cpus");
					for (int cpu = 0 ; cpu < value.cpus ; cpu++)
						if (value.numaMap[cpu] == node || value.numaMap[cpu] == -1)
							json.printValue(cpu);
				json.closeFieldArray("cpus");
			json.closeFieldStruct(nodeName);
		}
	json.closeStruct();
}

}
