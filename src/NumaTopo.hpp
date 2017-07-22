/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef NUMA_TOPO_HPP
#define NUMA_TOPO_HPP

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/********************  CLASS  **********************/
class NumaTopo
{
    public:
        NumaTopo(void);
        ~NumaTopo(void);
    private:
        void loadCpuNb(void);
        void loadNumaMap(void);
    private:
        int cpus;
        int numaNodes;
        int * numaMap;

};

}

#endif //NUMA_TOPO_HPP
