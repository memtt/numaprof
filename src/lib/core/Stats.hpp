/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef NUMAPROF_STATS_HPP
#define NUMAPROF_STATS_HPP

/********************  MACRO  ***********************/
//#define NUMAPROF_CALLSTACK

/*******************  HEADERS  **********************/
#include <cstdlib>
#include <map>
#include "Stack.hpp"
#include "../../../extern-deps/from-htopml/json/ConvertToJson.h"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  STRUCT  *********************/
/**
 * Stats counters to be attached to each instructions and call sites.
**/
struct Stats
{
	//functions
	Stats(void);
	void merge(Stats & info);
	void applyCut(float cutoff);
	bool asOneLargerThan(Stats & info);

	//members
	/** Count how many pages have been first touched by a pinned thread **/
	size_t firstTouch;
	/** Count how many pages have been first touched by a non pinned thread **/
	size_t unpinnedFirstTouch;
	/** Count how accesses have been done on unpinned pages **/
	size_t unpinnedPageAccess;
	/** Count how many pages have been done in a non binned thread **/
	size_t unpinnedThreadAccess;
	/** Count how many page have been done in a non pinned thread for a non pinned page. **/
	size_t unpinnedBothAccess;
	/** Count the local memory accesses **/
	size_t localAccess;
	/** Count the remote memory acesses **/
	size_t remoteAccess;
	/** Count the memory accesses to the Intel KNL MCDRAM **/
	size_t localMcdramAccess;
	/** Count the memory accesses to the Intel KNL MCDRAM **/
	size_t remoteMcdramAccess;
	/** Count hte memory accesses to non dyanmically allocated objects (stack, global varaibles, consts) **/
	size_t nonAlloc;
};

/*********************  TYPES  **********************/
/**Define the instruction map index. **/
#ifdef NUMAPROF_CALLSTACK
typedef std::map<MiniStack,Stats> InstrInfoMap;
#else
typedef std::map<size_t,Stats> InstrInfoMap;
#endif

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const Stats& value);
void convertToJson(htopml::JsonState& json, const InstrInfoMap& value);

/*******************  FUNCTION  *********************/
/**
 * Check if one of the value is larger than the ref. This function is
 * used by the cutoff function (removeSmall) to filter the instructions
 * we keep in the final profile.
 * @param info Define the reference to compare with.
**/
inline bool Stats::asOneLargerThan(Stats & info)
{
	//check if one is biffer than ref
	if (firstTouch > info.firstTouch)
		return true;
	if (unpinnedFirstTouch > info.unpinnedFirstTouch)
		return true;
	if (unpinnedPageAccess > info.unpinnedPageAccess)
		return true;
	if (unpinnedThreadAccess > info.unpinnedThreadAccess)
		return true;
	if (unpinnedBothAccess > info.unpinnedBothAccess)
		return true;
	if (localAccess > info.localAccess)
		return true;
	if (remoteAccess > info.remoteAccess)
		return true;
	if (localMcdramAccess > info.localMcdramAccess)
		return true;
	if (remoteMcdramAccess > info.remoteMcdramAccess)
		return true;
	if (nonAlloc > info.nonAlloc)
		return true;

	//none
	return false;
}

}

#endif //NUMAPROF_STATS_HPP
