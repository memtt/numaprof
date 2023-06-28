/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

//Imported from mpc_allocator_cpp : https://github.com/svalat/mpc_allocator_cpp
//from same author.

/********************  HEADERS  *********************/
#include "TopoHwloc.hpp"
#include "Debug.hpp"
#include <hwloc.h>

/********************  NAMESPACE  *******************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
TopoHwloc::TopoHwloc ( void )
{
	//vars
	int status;
	
	//init hwloc
	status = hwloc_topology_init(&topology);
	numaprofAssume(status == 0,"Failed to init hwloc.");
	
	//load current topology
	status = hwloc_topology_load(topology);
	numaprofAssume(status == 0,"Failed to load current topology with hwloc.");
}

/*******************  FUNCTION  *********************/
TopoHwloc::~TopoHwloc ( void )
{
	hwloc_topology_destroy(topology);
}

/*******************  FUNCTION  *********************/
void TopoHwloc::loadTopologyFromFile ( const char* filename )
{
	//vars
	int status;

	//destroy the old topology
	hwloc_topology_destroy(topology);
	
	//load the new def
	status = hwloc_topology_set_xml(topology,filename);
	numaprofAssume(status == 0,"Failed to load topological information from file ... in hwloc.");//,filename);
	
	//load current topology
	status = hwloc_topology_load(topology);
	numaprofAssume(status == 0,"Failed to apply topology from ... with hwloc.");//,filename);
}

/*******************  FUNCTION  *********************/
/*int TopoHwloc::getLevelId ( const char* name ) const
{
	//convert
	hwloc_obj_type_t res = hwloc_obj_type_of_string(name);
	numaprofAssume(res != -1,"Invalid topological level name ... for hwloc.");//name
	
	return res;
}*/

/*******************  FUNCTION  *********************/
const char* TopoHwloc::getLevelName ( int id ) const
{
	const char * res = hwloc_obj_type_string((hwloc_obj_type_t)id);
	numaprofAssume(res != NULL,"Failed to conver topological level id ... from hwloc.");//id
	
	return res;
}

/*******************  FUNCTION  *********************/
int TopoHwloc::getNbEntities ( int level, int depth ) const
{
	//get real depth
	int absDepth = getAbsDepth(level,depth);
	numaprofAssume(depth != -1,"Cannot find the depth for the requested level, depth couple in hwloc.");//level,depth
	
	//get number of objs
	int res = hwloc_get_nbobjs_by_depth(topology, absDepth);
	numaprofAssume(res > 0, "Invalid nbobjs_by_depth in hwloc for absolute depth ....");//absDepth
	return res;
}

/*******************  FUNCTION  *********************/
int TopoHwloc::getNbNumaEntities ( void ) const
{
	//get number of objs
	int res = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_NODE);
	numaprofAssume(res >= 0, "Invalid nbobjs_by_depth in hwloc for absolute depth ....");//absDepth
	return res;
}

/*******************  FUNCTION  *********************/
int TopoHwloc::getCurentNumaId(void) const
{
	//vars
	int res = -1;

	//try to find by using NUMA bindings
	res = getCurrentIdFromNUMABinding();

	//if not found try to find with thread binding on cores
	if (res == -1)
		res = getCurrentIdFromThreadBinding();

	return res;
}

/*******************  FUNCTION  *********************/
int TopoHwloc::getCurrentIdFromNUMABinding(void) const
{
	hwloc_nodeset_t nodeset = hwloc_bitmap_alloc();
	hwloc_cpuset_t cpuset = hwloc_bitmap_alloc();
	hwloc_membind_policy_t policy;
	int res = -1;
	int weight;
	int status;
	#if defined(SCTK_ALLOC_DEBUG) && defined(hwloc_bitmap_list_snprintf)
	char buffer[4096];
	#endif

	//if no numa node, return immediately
	if (getNbNumaEntities() == 1)
		return -1;

	//nodes
	// flags = 0 fallback on PROCESS if THREAD is not supported (as for windows).
	status =  hwloc_get_membind_nodeset(topology,nodeset,&policy,0);
	assert(status == 0);
	if (status == 0)
		return -1;

	#if defined(SCTK_ALLOC_DEBUG) && defined(hwloc_bitmap_list_snprintf)
	status = hwloc_bitmap_list_snprintf(buffer,4096,nodeset);
	sprintf(stderr,"Current nodes : %s\n",buffer);
	#endif

	//cores
	// flags = 0 fallback on PROCESS if THREAD is not supported (as for windows).
	status =  hwloc_get_membind(topology,cpuset,&policy,0);
	assert(status == 0);
	if (status == 0)
		return -1;

	#if defined(SCTK_ALLOC_DEBUG) && defined(hwloc_bitmap_list_snprintf)
	status = hwloc_bitmap_list_snprintf(buffer,4096,cpuset);
	sprintf(stderr,"Current cores : %s\n",buffer);
	#endif

	//nodes from cores
	hwloc_cpuset_to_nodeset(topology,cpuset,nodeset);

	#if defined(SCTK_ALLOC_DEBUG) && defined(hwloc_bitmap_list_snprintf)
	status = hwloc_bitmap_list_snprintf(buffer,4096,nodeset);
	sprintf(stderr,"Current nodes from cores : %s\n",buffer);
	#endif

	//calc res
	weight = hwloc_bitmap_weight(nodeset);
	assert(weight != 0);
	if (weight == 1)
		res = getFirstBitInBitmap(nodeset);

	hwloc_bitmap_free(cpuset);
	hwloc_bitmap_free(nodeset);

	return res;
}

/*******************  FUNCTION  *********************/
int TopoHwloc::getCurrentIdFromThreadBinding(void) const
{
	hwloc_nodeset_t nodeset = hwloc_bitmap_alloc();
	hwloc_cpuset_t cpuset = hwloc_bitmap_alloc();
	int res = -1;
	int weight;
	#if defined(SCTK_ALLOC_DEBUG) && defined(hwloc_bitmap_list_snprintf)
	char buffer[4096];
	#endif
	
	//get current core binding
	//for windows use 0 instead of HWLOC_CPUBIND_THREAD
	int status = hwloc_get_cpubind (topology, cpuset, 0);
	assert(status == 0);
	if (status == 0)
		return -1;

	#if defined(SCTK_ALLOC_DEBUG) && defined(hwloc_bitmap_list_snprintf)
	status = hwloc_bitmap_list_snprintf(buffer,4096,cpuset);
	sprintf(stderr,"Current cores : %s\n",buffer);
	#endif

	//nodes from cores
	hwloc_cpuset_to_nodeset(topology,cpuset,nodeset);

	#if defined(SCTK_ALLOC_DEBUG) && defined(hwloc_bitmap_list_snprintf)
	status = hwloc_bitmap_list_snprintf(buffer,4096,nodeset);
	sprintf(stderr,"Current nodes from cores : %s\n",buffer);
	#endif

	//calc res
	weight = hwloc_bitmap_weight(nodeset);
	assert(weight != 0);
	if (weight == 1)
		res = getFirstBitInBitmap(nodeset);

	hwloc_bitmap_free(cpuset);
	hwloc_bitmap_free(nodeset);

	return res;
}

/*******************  FUNCTION  *********************/
int TopoHwloc::getFirstBitInBitmap(hwloc_bitmap_t bitmap) const
{
	int last = hwloc_bitmap_last(bitmap);
	int current = hwloc_bitmap_first(bitmap);
	assert(current != -1);
	while (current != last)
	{
		if (hwloc_bitmap_isset(bitmap,current))
			break;
		current = hwloc_bitmap_next(bitmap,current);
	}
	return current;
}

/*******************  FUNCTION  *********************/
int TopoHwloc::getAbsDepth ( int level, int depth ) const
{
	//vars
	int curDepth = 0;
	
	//get max depth
	int topodepth = hwloc_topology_get_depth(topology);
	
	for (int i = 0 ; i < topodepth ; i++)
	{
		//get current type
		hwloc_obj_type_t type = hwloc_get_depth_type(topology,i);
		if (type == level)
		{
			curDepth++;
			if (curDepth == depth)
				return i;
		}
	}
	
	//error
	numaprofFatal("Fail to find the depth of requested level levelType=%d, depth=%d in hwloc.",level,depth);
	return -1;
}

/*******************  FUNCTION  *********************/
int TopoHwloc::getCurrentId ( int level, int depth ) const
{
	//vars
	int res = -1;

	//get absolute depth
	int absDepth = getAbsDepth(level,depth);
	
	//search current thread binding
	hwloc_cpuset_t cpuset = hwloc_bitmap_alloc();
	int status = hwloc_get_cpubind (topology, cpuset, 0);
	numaprofAssume(status >= 0,"Failed to get the current thread CPU binding with hwloc.");
	
	//count overlap over current level
	int cnt = hwloc_get_nbobjs_inside_cpuset_by_depth(topology,cpuset,absDepth);
	
	//if get only one return its ID
	if (cnt == 1)
	{
		hwloc_obj_t obj = hwloc_get_obj_inside_cpuset_by_depth(topology,cpuset,absDepth,0);
		//TODO maybe their is an accessor for this
		res = obj->logical_index;
	}
	
	//clean memory
	hwloc_bitmap_free(cpuset);
	
	return res;
}

};
