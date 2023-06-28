/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef MUTEX_HPP
#define MUTEX_HPP

/********************  HEADERS  *********************/
#ifdef USE_PIN_LOCKS
	#include "pin.H"
#elif defined(USE_DIRECT_PTHREAD_LOCKS)
	#include <pthread.h>
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
#elif defined(USE_DIRECT_PTHREAD_LOCKS)
	class Mutex
	{
		public:
			Mutex(void) {pthread_mutex_init(&mutex, NULL);}
			void lock(void) { pthread_mutex_lock(&mutex); }
			void unlock(void) { pthread_mutex_unlock(&mutex); }
		private:
			pthread_mutex_t mutex;
	};
#else
	#include <mutex>
	
	typedef std::mutex Mutex;
#endif

}

#endif //MUTEX_HPP
