/*****************************************************
			 PROJECT  : numaprof
			 VERSION  : 0.0.0-dev
			 DATE     : 05/2017
			 AUTHOR   : Valat Sébastien - CERN
			 LICENSE  : CeCILL-C
*****************************************************/

//Imported from libnuma to not have to link to libnuma inside pintool

#ifndef NUMAIF_H
#define NUMAIF_H 1

#include <cstdlib>

// If keep move_pages it generates symbol error on last pin version (3.24), so use another symbol name to avoid issues.
#define move_pages move_pages_pintool

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
long move_pages(int pid, unsigned long count,void **pages, const int *nodes, int *status, int flags);
long get_mempolicy(int *policy, unsigned long *nmask,unsigned long maxnode, void *addr,unsigned flags);
long set_mempolicy(int mode, const unsigned long *nmask,unsigned long maxnode);

}

#endif //NUMAIF_H
