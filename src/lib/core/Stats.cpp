/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include "Stats.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
/**
 * This is more a check function for validation, it does an atomic add
 * and check if the sum overflow and print warning if it append
 * @param out Reference to the output number
 * @param in what to add to the output number.
**/
static inline void addAndCheckOverflow(uint64_t & out,uint64_t in)
{
	uint64_t before = out;
	uint64_t after = __sync_fetch_and_add(&out,in,__ATOMIC_RELAXED);
	if (after < before)
	{
		fprintf(stderr,"NUMAPROF: counter overflow !\n");
		abort();
	}
}

/*******************  FUNCTION  *********************/
/**
 * Constructor of the statistic structure. It init all the fields to 0.
**/
Stats::Stats(void)
{
	firstTouch = 0;
	unpinnedFirstTouch = 0;
	unpinnedPageAccess = 0;
	unpinnedThreadAccess = 0;
	unpinnedBothAccess = 0;
	localAccess = 0;
	remoteAccess = 0;
	localMcdramAccess = 0;
	remoteMcdramAccess = 0;
	nonAlloc = 0;
}

/*******************  FUNCTION  *********************/
/**
 * Merge two stats structure by using atomic add on all fields.
 * @param info Structure to add to the current one.
**/
void Stats::merge(Stats & info)
{
	__sync_fetch_and_add(&this->firstTouch,info.firstTouch,__ATOMIC_RELAXED);
	__sync_fetch_and_add(&this->unpinnedFirstTouch,info.unpinnedFirstTouch,__ATOMIC_RELAXED);
	__sync_fetch_and_add(&this->unpinnedPageAccess,info.unpinnedPageAccess,__ATOMIC_RELAXED);
	__sync_fetch_and_add(&this->unpinnedThreadAccess,info.unpinnedThreadAccess,__ATOMIC_RELAXED);
	__sync_fetch_and_add(&this->unpinnedBothAccess,info.unpinnedBothAccess,__ATOMIC_RELAXED);
	__sync_fetch_and_add(&this->localAccess,info.localAccess,__ATOMIC_RELAXED);
	__sync_fetch_and_add(&this->remoteAccess,info.remoteAccess,__ATOMIC_RELAXED);
	__sync_fetch_and_add(&this->localMcdramAccess,info.localMcdramAccess,__ATOMIC_RELAXED);
	__sync_fetch_and_add(&this->remoteMcdramAccess,info.remoteMcdramAccess,__ATOMIC_RELAXED);
	__sync_fetch_and_add(&this->nonAlloc,info.nonAlloc,__ATOMIC_RELAXED);
	
	/*addAndCheckOverflow(this->firstTouch,info.firstTouch);
	addAndCheckOverflow(this->unpinnedFirstTouch,info.unpinnedFirstTouch);
	addAndCheckOverflow(this->unpinnedPageAccess,info.unpinnedPageAccess);
	addAndCheckOverflow(this->unpinnedThreadAccess,info.unpinnedThreadAccess);
	addAndCheckOverflow(this->unpinnedBothAccess,info.unpinnedBothAccess);
	addAndCheckOverflow(this->localAccess,info.localAccess);
	addAndCheckOverflow(this->remoteAccess,info.remoteAccess);
	addAndCheckOverflow(this->localMcdramAccess,info.localMcdramAccess);
	addAndCheckOverflow(this->remoteMcdramAccess,info.remoteMcdramAccess);
	addAndCheckOverflow(this->nonAlloc,info.nonAlloc);*/
}

/*******************  FUNCTION  *********************/
/**
 * Apply a ratio to compute the cutoff value to cut small values.
 * @param cutoff Cutoff in percent.
**/
void Stats::applyCut(float cutoff)
{
	//compute real cutoff from %
	cutoff /= 100.0;
	
	//apply
	firstTouch = ((float)firstTouch) * cutoff;
	unpinnedFirstTouch = ((float)unpinnedFirstTouch) * cutoff;
	unpinnedPageAccess = ((float)unpinnedPageAccess) * cutoff;
	unpinnedThreadAccess = ((float)unpinnedThreadAccess)* cutoff;
	unpinnedBothAccess = ((float)unpinnedBothAccess) *cutoff;
	localAccess = ((float)localAccess)*cutoff;
	remoteAccess = ((float)remoteAccess)*cutoff;
	localMcdramAccess = ((float)localMcdramAccess)*cutoff;
	remoteMcdramAccess = ((float)remoteMcdramAccess)*cutoff;
	nonAlloc = ((float)nonAlloc)*cutoff;
}

/*******************  FUNCTION  *********************/
/**
 * Function to convert the stats structu into JSON format.
 * @param json JSON state to output data.
 * @param value Object to convert.
**/
void convertToJson(htopml::JsonState& json, const Stats& value)
{
	json.openStruct();
		if (value.firstTouch > 0)
			json.printField("firstTouch",value.firstTouch);
		if (value.unpinnedFirstTouch > 0)
			json.printField("unpinnedFirstTouch",value.unpinnedFirstTouch);
		if (value.unpinnedPageAccess > 0)
			json.printField("unpinnedPageAccess",value.unpinnedPageAccess);
		if (value.unpinnedThreadAccess > 0)
			json.printField("unpinnedThreadAccess",value.unpinnedThreadAccess);
		if (value.unpinnedBothAccess > 0)
			json.printField("unpinnedBothAccess",value.unpinnedBothAccess);
		if (value.localAccess > 0)
			json.printField("localAccess",value.localAccess);
		if (value.remoteAccess > 0)
			json.printField("remoteAccess",value.remoteAccess);
		if (value.localMcdramAccess > 0)
			json.printField("localMcdramAccess",value.localMcdramAccess);
		if (value.remoteMcdramAccess > 0)
			json.printField("remoteMcdramAccess",value.remoteMcdramAccess);
		if (value.nonAlloc > 0)
			json.printField("nonAlloc",value.nonAlloc);
	json.closeStruct();
}

/*******************  FUNCTION  *********************/
/**
 * Function to convert the stats structu into JSON format.
 * @param json JSON state to output data.
 * @param value Object to convert.
**/
void convertToJson(htopml::JsonState& json, const InstrInfoMap& value)
{
	#ifdef NUMAPROF_CALLSTACK
		json.openArray();
			for (InstrInfoMap::const_iterator it = value.begin() ; it != value.end() ; ++it)
			{
				json.printListSeparator();
				json.openStruct();
					json.printField("stack",it->first);
					json.printField("stats",it->second);
				json.closeStruct();
			}
		json.closeArray();
	#else
		json.openStruct();
			for (InstrInfoMap::const_iterator it = value.begin() ; it != value.end() ; ++it)
			{
				char buffer[32];
				sprintf(buffer,"0x%lx",it->first);
				json.printField(buffer,it->second);
			}
		json.closeStruct();
	#endif
}

}
