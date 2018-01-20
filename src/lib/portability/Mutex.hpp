/*****************************************************
             PROJECT  : numaprof
             VERSION  : 0.0.0-dev
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef MUTEX_HPP
#define MUTEX_HPP

/********************  HEADERS  *********************/
#ifdef USE_PIN_LOCKS
	#include "pin.H"
#else
	#include <mutex>
#endif

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/********************  CLASS  ***********************/
#ifdef USE_PIN_LOCKS
	class Mutex
	{
		public:
			void lock(void) { PIN_GetLock(&mutex, -1); };
			void unlock(void) { PIN_ReleaseLock(&mutex); };
		private:
			PIN_LOCK mutex;
	};
#else
	#include <mutex>
	
	typedef std::mutex Mutex;
#endif

}

#endif //MUTEX_HPP
