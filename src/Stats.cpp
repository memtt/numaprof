/*****************************************************
			 PROJECT  : numaprof
			 VERSION  : 2.3.0
			 DATE     : 05/2017
			 AUTHOR   : Valat Sébastien - CERN
			 LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include "Stats.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
Stats::Stats(void)
{
	firstTouch = 0;
	unpinnedFirstTouch = 0;
	unpinnedPageAccess = 0;
	unpinnedThreadAccess = 0;
	unpinnedBothAccess = 0;
	localAccess = 0;
	remoteAccess = 0;
}

/*******************  FUNCTION  *********************/
void Stats::merge(Stats & info)
{
	this->firstTouch += info.firstTouch;
	this->unpinnedFirstTouch += info.unpinnedFirstTouch;
	this->unpinnedPageAccess += info.unpinnedPageAccess;
	this->unpinnedThreadAccess += info.unpinnedThreadAccess;
	this->unpinnedBothAccess += info.unpinnedBothAccess;
	this->localAccess += info.localAccess;
	this->remoteAccess += info.remoteAccess;
}

/*******************  FUNCTION  *********************/
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
		if (value.mcdramAccess > 0)
			json.printField("mcdramAccess",value.mcdramAccess);
	json.closeStruct();
}

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const InstrInfoMap& value)
{
	json.openStruct();
		for (InstrInfoMap::const_iterator it = value.begin() ; it != value.end() ; ++it)
		{
			char buffer[32];
			sprintf(buffer,"0x%lx",it->first);
				json.printField(buffer,it->second);
		}
	json.closeStruct();
}

}
