/*****************************************************
			 PROJECT  : numaprof
			 VERSION  : 2.3.0
			 DATE     : 05/2017
			 AUTHOR   : Valat Sébastien - CERN
			 LICENSE  : CeCILL-C
*****************************************************/

#ifndef NUMAPROF_STATS_HPP
#define NUMAPROF_STATS_HPP

/*******************  HEADERS  **********************/
#include <cstdlib>
#include "../extern-deps/from-htopml/json/ConvertToJson.h"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  STRUCT  *********************/
struct Stats
{
	//functions
	Stats(void);
	void merge(Stats & info);

	//members
	size_t firstTouch;
	size_t unpinnedFirstTouch;
	size_t unpinnedPageAccess;
	size_t unpinnedThreadAccess;
	size_t unpinnedBothAccess;
	size_t localAccess;
	size_t remoteAccess;
};

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const Stats& value);

}

#endif //NUMAPROF_STATS_HPP
