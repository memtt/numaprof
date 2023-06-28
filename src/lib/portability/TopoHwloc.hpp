/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

//Imported from mpc_allocator_cpp : https://github.com/svalat/mpc_allocator_cpp
//from same author.

#ifndef NUMAPROF_TOPO_HWLOC_H
#define NUMAPROF_TOPO_HWLOC_H

/********************  HEADERS  *********************/
#include <hwloc.h>

/********************  NAMESPACE  *******************/
namespace numaprof
{

/*********************  CLASS  **********************/
class TopoHwloc
{
	public:
		//lifecycle
		TopoHwloc(void);
		~TopoHwloc(void);
		//get info
		int getLevelId(const char * name) const;
		const char * getLevelName(int id) const;
		int getNbNumaEntities(void) const;
		int getNbEntities(int level,int depth) const;
		int getCurrentId(int level, int depth) const;
		int getCurentNumaId(void) const;
		int getCurrentIdFromNUMABinding(void) const;
		int getCurrentIdFromThreadBinding(void) const;
		//load predefined topo
		void loadTopologyFromFile(const char * filename);
	private:
		//some helper methods
		int getAbsDepth(int level,int depth) const;
		int getFirstBitInBitmap(hwloc_bitmap_t bitmap) const;
	private:
		hwloc_topology_t topology;
};

};

#endif //NUMAPROF_TOPO_HWLOC_H
