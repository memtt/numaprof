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
	private:
		ProcessTracker * process;
		InstrInfoMap instructions;
		InstrInfo stats;
		int numa;
		PageTable * table;
};

}

#endif //THREAD_TRACKER_HPP
