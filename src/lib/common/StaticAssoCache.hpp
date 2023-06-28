/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef NUMAPROF_STATIC_ASSO_CACHE_HPP
#define NUMAPROF_STATIC_ASSO_CACHE_HPP

/********************  HEADERS  *********************/
//std
#include <cstdlib>

/********************  MACRO  ***********************/
// #define NUMAPROF_CACHE_STATS

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  CLASS  **********************/
/**
 * Implement a cache to store datas identified by addresses. The cache is built
 * as an associative cache with N ways and M rows per way. It is built in template
 * to be faster and optimized by compiler.
**/
template<class T,int ways,int rows>
class StaticAssoCache
{
	public:
		StaticAssoCache(void);
		void flush(void);
		T * get(size_t addr) const;
		void set(size_t addr,T * value);
		void printStats(const char * name) const;
	private:
		/** Store content **/
		T * content[rows][ways];
		/** store addresses identifying content to match requests **/
		size_t addr[rows][ways];
		/** Used to round robin on rows to override **/
		unsigned char next[rows];
		/** count miss **/
		mutable size_t miss;
		/** counter hits **/
		mutable size_t hits;
};

}

/**********************  IMPL  **********************/
#include "StaticAssoCache_impl.hpp"

#endif //NUMAPROF_STATIC_ASSO_CACHE_HPP
