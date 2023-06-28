/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
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
#include "../common/Options.hpp"
#include "../portability/NumaTopo.hpp"
#include "../portability/OS.hpp"
#include "linux/mempolicy.h"
#ifdef __PIN__
	#include "../../extern-deps/from-numactl/MovePages.hpp"
#else
	#include "numaif.h"
#endif

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

#define HWLOC_BITS_PER_LONG (sizeof(unsigned long)*8)

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/********************  GLOBALS  *********************/
static const char * gblBindingNames[] = {
	"NO_BIND",
	"INTERLEAVE",
	"BIND_ONE",
	"BIND_MULTIPLE"
};

/*******************  FUNCTION  *********************/
NumaTopo::NumaTopo(void)
{
	this->cpus = 0;
	this->numaNodes = 0;
	this->numaMap = NULL;

	this->loadCpuNb();
	this->loadNumaMap();
	this->loadDistanceMap();
	this->loadParendNode();
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
	if (!getGlobalOptions().outputSilent)
		fprintf(stderr,"NUMAPROF: Get cpu number : %d\n",cpus);
}

/*******************  FUNCTION  *********************/
int NumaTopo::getDistanceMax(void) const
{
	int max = 0;
	for (int i = 0 ; i < numaNodes * numaNodes ; i++)
		if (distanceMap[i] > max)
			max = distanceMap[i];
	return max;
}

/*******************  FUNCTION  *********************/
void NumaTopo::loadDistanceMap(void)
{
	//check
	assert(numaNodes > 0);
	
	//allocate
	distanceMap = new int[numaNodes * numaNodes];
	memset(distanceMap,0,numaNodes*numaNodes*sizeof(int));
	
	//load
	//loop on all numa nodes (until we do not found one)
	int node = 0;
	while (true)
	{
		//build file path
		char fname[128];
		sprintf(fname,"/sys/devices/system/node/node%d/distance",node);

		//load
		char * list = OS::loadTxtFile(fname,256);
		if (list == NULL)
			break;
		
		//load line
		int index = 0;
		char * cur = list;
		while (*cur != '\0')
		{
			distanceMap[node*numaNodes+index] = atoi(cur);
			index++;
			
			//move to next
			while (*cur != '\0' && *cur != ' ')
				cur++;
			if (*cur == ' ')
				cur++;
		}
		
		//incr
		node++;
	}
	
	//get all number
	std::map<int,int> values;
	for (int i = 0 ; i <numaNodes * numaNodes ; i++)
		values[distanceMap[i]] = 0;
	
	//renumber
	int cnt = 0;
	for (std::map<int,int>::iterator it = values.begin() ; it != values.end() ; ++it)
		it->second = cnt++;
	
	//apply
	for (int i = 0 ; i <numaNodes * numaNodes ; i++)
		distanceMap[i] = values[distanceMap[i]];
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
			fprintf(stderr,"NUMAPROF: Caution, CPU %d was not assigned to a NUMA node, assigned to 0 !\n",i);
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
	numaprofAssume(sizeof(*mask) * 8 >= (size_t)cpus,"Size issue with CPU mask from CPUSET !");

	//clear
	if (cpuBindList != NULL)
		cpuBindList->clear();

	int cnt = size * 8;
	if (!getGlobalOptions().outputSilent)
		fprintf(stderr,"NUMAPROF: Size = %d, ncpu = %d\n",cnt,cpus);
	if (cnt > cpus)
		cnt = cpus;
	
	//check numa
	for (int i = 0 ; i < cnt ; i++)
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

	if (!getGlobalOptions().outputSilent)
		fprintf(stderr,"NUMAPROF: Thread is binded on NUMA %d\n",numa);
	return numa;
}

/*******************  FUNCTION  *********************/
//copied from hwloc topology-linux.c
/*
 * On some kernels, get_mempolicy requires the output size to be larger
 * than the kernel MAX_NUMNODES (defined by CONFIG_NODES_SHIFT).
 * Try get_mempolicy on ourself until we find a max_os_index value that
 * makes the kernel happy.
 */
size_t NumaTopo::findMemPolicyKernelSize(void)
{
	static int _max_numnodes = -1, max_numnodes;
	int linuxpolicy;

	if (_max_numnodes != -1)
		/* already computed */
		return _max_numnodes;

	/* start with a single ulong, it's the minimal and it's enough for most machines */
	max_numnodes = HWLOC_BITS_PER_LONG;
	while (1) {
		unsigned long *mask =(unsigned long*) malloc(max_numnodes / HWLOC_BITS_PER_LONG * sizeof(long));
		int err = get_mempolicy(&linuxpolicy, mask, max_numnodes, 0, 0);
		free(mask);
		if (!err || errno != EINVAL)
			/* Found it. Only update the static value with the final one,
			* to avoid sharing intermediate values that we modify,
			* in case there's ever multiple concurrent calls.
			*/
			return _max_numnodes = max_numnodes;
		max_numnodes *= 2;
	}
}

/*******************  FUNCTION  *********************/
MemPolicy NumaTopo::getCurrentMemPolicy()
{
	static bool hasMemPolicy = true;
	static int size = 0;
	
	if (size == 0)
		size = findMemPolicyKernelSize();

	MemPolicy policy;
	int status = 1;
	
	if (hasMemPolicy)
	{
		unsigned long *mask = (unsigned long*)malloc(size / HWLOC_BITS_PER_LONG * sizeof(long));
		status = get_mempolicy(&policy.mode,mask,size,NULL,0);
		if (status != 0)
		{
			fprintf(stderr,"\033[31mNUMAPROF: CAUTION, get_mempolicy not implemented, you might be running on a non NUMA system !\nAll accesses will be considered local !\033[0m\n");
			hasMemPolicy = false;
		} else {
			for (int i = 0 ; i < numaNodes ; i++)
				policy.mask[i/64] |= (mask[i/64] & (1lu << (i%64)));
		}
		free(mask);
	}

	if (status != 0)
	{
		policy.mode = MPOL_BIND;
		for (int i = 0 ; i < numaNodes ; i++)
			policy.mask[i/64] |= 1lu << (i%64);
	}
	
	staticComputeBindType(policy);
	return policy;
}

/*******************  FUNCTION  *********************/
void NumaTopo::staticComputeBindType(MemPolicy & policy)
{
	//default
	policy.type = MEMBIND_NO_BIND;
	
	//check numa
	int numa = -2;
	for (int i = 0 ; i < numaNodes ; i++)
	{
		if (policy.mask[i/64] & (1lu << (i%64)))
		{
			if (numa == -2)
				numa = numaMap[i];
			if (numa != numaMap[i])
				numa = -1;
		}
	}
	
	//no binding
	if (policy.mode == MPOL_DEFAULT || policy.mode == MPOL_LOCAL)
	{
		if (numa >= 0)
			policy.type = MEMBIND_BIND_ONE;
		else
			policy.type = MEMBIND_NO_BIND;
	}
	
	//consider as bind but give an undefined value
	if (policy.mode == MPOL_INTERLEAVE)
		policy.type = MEMBIND_INTERLEAVE;

	//binding, check which node is prefered.
	if (policy.mode == MPOL_PREFERRED || policy.mode == MPOL_BIND)
	{
		if (numa == -2 || numa >= 0)
			policy.type = MEMBIND_BIND_ONE;
		if (numa == -1)
			policy.type = MEMBIND_BIND_MULTIPLE;
	}
}

/*******************  FUNCTION  *********************/
int NumaTopo::getNumaNodes ( void )
{
	return this->numaNodes;
}

/*******************  FUNCTION  *********************/
void NumaTopo::loadParendNode(void)
{
	//allocate
	this->parentNode = new int[numaNodes];
	
	//loop on all nodes
	for (int i = 0 ; i < numaNodes ; i++)
	{
		//search lower distance
		int distanceMin = -1;
		int parent = -1;
		for (int j = 0 ; j < numaNodes ; j++)
		{
			if ((getDistance(i,j) < distanceMin || distanceMin == -1) && isMcdram[j] == false)
			{
				distanceMin = getDistance(i,j);
				parent = j;
			}
		}
		
		//warning
		if (parent == -1)
		{
			numaprofWarning("Fail to find parent of MCDRAM NUMA node, use 0 !");
			parent = 0;
		}

		//store
		this->parentNode[i] = parent;
	}
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
				json.openFieldArray("distance");
					for (int i = 0 ; i < value.numaNodes ; i++)
						json.printValue(value.distanceMap[node*value.numaNodes+i]);
				json.closeFieldArray("distance");
			json.closeFieldStruct(nodeName);
		}
	json.closeStruct();
}

/*******************  FUNCTION  *********************/
const char * getMemBindTypeName(MemBindType type)
{
	return gblBindingNames[type];
}

}
