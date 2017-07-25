/*****************************************************
			 PROJECT  : numaprof
			 VERSION  : 2.3.0
			 DATE     : 05/2017
			 AUTHOR   : Valat Sébastien - CERN
			 LICENSE  : CeCILL-C
*****************************************************/

#ifndef THREAD_TRACKER_HPP
#define THREAD_TRACKER_HPP

/********************  HEADERS  *********************/
#include <map>
#include "../extern-deps/from-htopml/json/ConvertToJson.h"
#include "ProcessTracker.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  CLASS  **********************/
class ThreadTracker
{
	public:
		ThreadTracker(ProcessTracker * process);
		void flush(void);
		void onAccess(size_t ip,size_t addr,bool write);
		void onSetAffinity(cpu_set_t * mask);
		void onStop(void);
		void onMunmap(size_t addr,size_t size);
		friend void convertToJson(htopml::JsonState& json, const ThreadTracker& value);
	private:
		ProcessTracker * process;
		InstrInfoMap instructions;
		Stats stats;
		int numa;
		PageTable * table;
};

}

#endif //THREAD_TRACKER_HPP
