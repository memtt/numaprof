/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
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
#include "../../extern-deps/from-htopml/json/ConvertToJson.h"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/********************  TYPES  ***********************/
typedef std::list<int> CpuBindList;

/********************  CLASS  ***********************/
class NumaTopo
{
	public:
		NumaTopo(void);
		~NumaTopo(void);
		int getCurrentNumaAffinity(CpuBindList * cpuBindList = NULL);
		int getCurrentNumaAffinity(cpu_set_t * mask,CpuBindList * cpuBindList = NULL);
		bool getIsMcdram(int cpuid) {assert(cpuid < cpus);return isMcdram[cpuid];};
		friend void convertToJson(htopml::JsonState& json, const NumaTopo& value);
		int getNumaNodes(void);
	private:
		void loadCpuNb(void);
		void loadNumaMap(void);
	private:
		int cpus;
		int numaNodes;
		int * numaMap;
		bool * isMcdram;

};

}

#endif //NUMA_TOPO_HPP
