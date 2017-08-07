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
#include <sys/types.h>
#include "../../extern-deps/from-htopml/json/ConvertToJson.h"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/********************  CLASS  **********************/
class NumaTopo
{
	public:
		NumaTopo(void);
		~NumaTopo(void);
		int getCurrentNumaAffinity(void);
		int getCurrentNumaAffinity(cpu_set_t * mask);
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
