/*****************************************************
             PROJECT  : numaprof
             VERSION  : 0.0.0-dev
             DATE     : 05/2017
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
#include "../../extern-deps/from-htopml/json/ConvertToJson.h"

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
		friend void convertToJson(htopml::JsonState& json, const NumaTopo& value);
		int getNumaNodes(void);
		MemPolicy getCurrentMemPolicy(void);
		void staticComputeBindType(MemPolicy & policy);
	private:
		void loadCpuNb(void);
		void loadNumaMap(void);
		void loadDistanceMap(void);
		size_t findMemPolicyKernelSize(void);
	private:
		int cpus;
		int numaNodes;
		int * numaMap;
		bool * isMcdram;
		int * distanceMap;
};

/*******************  FUNCTION  *********************/
const char * getMemBindTypeName(MemBindType type);

}

#endif //NUMA_TOPO_HPP
