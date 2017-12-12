/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
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
#include "../../extern-deps/from-htopml/json/ConvertToJson.h"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  STRUCT  *********************/
struct Stats
{
	//functions
	Stats(void);
	void merge(Stats & info);
	void applyCut(float cutoff);
	bool asOneLargerThan(Stats & info);

	//members
	size_t firstTouch;
	size_t unpinnedFirstTouch;
	size_t unpinnedPageAccess;
	size_t unpinnedThreadAccess;
	size_t unpinnedBothAccess;
	size_t localAccess;
	size_t remoteAccess;
	size_t mcdramAccess;
	size_t nonAlloc;
};

/*********************  TYPES  **********************/
#ifdef NUMAPROF_CALLSTACK
typedef std::map<MiniStack,Stats> InstrInfoMap;
#else
typedef std::map<size_t,Stats> InstrInfoMap;
#endif

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const Stats& value);
void convertToJson(htopml::JsonState& json, const InstrInfoMap& value);

/*******************  FUNCTION  *********************/
inline bool Stats::asOneLargerThan(Stats & info)
{
	//check if one is biffer than ref
	if (firstTouch >= info.firstTouch)
		return true;
	if (unpinnedFirstTouch >= info.unpinnedFirstTouch)
		return true;
	if (unpinnedPageAccess >= info.unpinnedPageAccess)
		return true;
	if (unpinnedThreadAccess >= info.unpinnedThreadAccess)
		return true;
	if (unpinnedBothAccess >= info.unpinnedBothAccess)
		return true;
	if (localAccess >= info.localAccess)
		return true;
	if (remoteAccess >= info.remoteAccess)
		return true;
	if (mcdramAccess >= info.mcdramAccess)
		return true;
	if (nonAlloc >= info.nonAlloc)
		return true;
	
	//none
	return false;
}

}

#endif //NUMAPROF_STATS_HPP
