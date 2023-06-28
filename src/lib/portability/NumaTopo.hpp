/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef NUMA_TOPO_HPP
#define NUMA_TOPO_HPP

#include <cassert>
#include <sched.h>
#include <list>
#include <sys/types.h>
#include "OS.hpp"
#include "../../../extern-deps/from-htopml/json/ConvertToJson.h"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/********************  TYPES  ***********************/
typedef std::list<int> CpuBindList;

/********************  ENUM  ************************/
enum MemBindType
{
	MEMBIND_NO_BIND,
	MEMBIND_INTERLEAVE,
	MEMBIND_BIND_ONE,
	MEMBIND_BIND_MULTIPLE
};

/*********************  STRUCT  *********************/
struct MemPolicy
{
	MemPolicy(void) {mask[0] = mask[1]= mask[2] = mask[3] = 0; type = MEMBIND_NO_BIND;mode = 0;};
	int mode;
	unsigned long mask[4];
	MemBindType type;
};

/********************  CLASS  ***********************/
class NumaTopo
{
	public:
		NumaTopo(void);
		~NumaTopo(void);
		int getCurrentNumaAffinity(CpuBindList * cpuBindList = NULL);
		int getCurrentNumaAffinity(cpu_set_t * mask,int size,CpuBindList * cpuBindList = NULL);
		bool getIsMcdram(int cpuid) {assert(cpuid < cpus);return isMcdram[cpuid];};
		int getParentNode(int numa) {assert(numa < numaNodes); return parentNode[numa];};
		friend void convertToJson(htopml::JsonState& json, const NumaTopo& value);
		int getNumaNodes(void);
		MemPolicy getCurrentMemPolicy(void);
		void staticComputeBindType(MemPolicy & policy);
		int getDistance(int source,int dest) const;
		int getDistanceMax(void) const;
	private:
		void loadCpuNb(void);
		void loadNumaMap(void);
		void loadDistanceMap(void);
		void loadParendNode(void);
		size_t findMemPolicyKernelSize(void);
	private:
		int cpus;
		int numaNodes;
		int * numaMap;
		bool * isMcdram;
		int * distanceMap;
		int * parentNode;
};

/*******************  FUNCTION  *********************/
const char * getMemBindTypeName(MemBindType type);

/*******************  FUNCTION  *********************/
inline int NumaTopo::getDistance(int source,int dest) const
{
	//check
	assert(source < numaNodes);
	assert(dest < numaNodes);

	//if one if not bound
	if (source == -1 || dest == -1)
		return -1;

	//check
	int id = source*numaNodes+dest;
	assert(id >= 0);
	assert(id < numaNodes*numaNodes);

	//get
	return distanceMap[id];
}

}

#endif //NUMA_TOPO_HPP
