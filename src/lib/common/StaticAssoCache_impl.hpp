/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef NUMAPROF_STATIC_ASSO_CACHE_IMPL_HPP
#define NUMAPROF_STATIC_ASSO_CACHE_IMPL_HPP

/********************  HEADERS  *********************/
//std
#include <cstdio>
#include <cassert>
#include "StaticAssoCache.hpp"

/********************  HEADERS  *********************/

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
/**
 * Constructor of the cache, it reset all the content of the cache.
**/
template<class T,int ways,int rows>
StaticAssoCache<T,ways,rows>::StaticAssoCache(void)
{
	assert(ways < 256);
	flush();
}

/*******************  FUNCTION  *********************/
template<class T,int ways,int rows>
void StaticAssoCache<T,ways,rows>::flush(void)
{
	for (int r = 0 ; r < rows ; r++)
	{
		next[r] = 0;
		for (int w = 0 ; w < ways ; w++)
		{
			content[r][w] = NULL;
			addr[r][w] = -1UL;
		}
	}
}

/*******************  FUNCTION  *********************/
/**
 * Look in the cache and return the corresponding address.
**/
template<class T,int ways,int rows>
T * StaticAssoCache<T,ways,rows>::get(size_t addr) const
{
	//compute row
	int r = addr % rows;

	//loop on ways
	for (int w = 0 ; w < ways ; w++)
	{
		if (this->addr[r][w] == addr)
		{
			//stats
			#ifdef NUMAPROF_CACHE_STATS
				hits++;
			#endif

			//ok
			return content[r][w];
		}
	}
	
	//stats
	#ifdef NUMAPROF_CACHE_STATS
		miss++;
	#endif

	//not in cache
	return NULL;
}

/*******************  FUNCTION  *********************/
/**
 * Look in the cache and return the corresponding address.
**/
template<class T,int ways,int rows>
void StaticAssoCache<T,ways,rows>::set(size_t addr, T * value)
{
	//compute row
	int r = addr % rows;
	
	//setup
	int w = next[r];
	this->content[r][w] = value;
	this->addr[r][w] = addr;
	
	//increment with round robin
	next[r] = (w+1)%ways;
}

/*******************  FUNCTION  *********************/
/**
 * Constructor of the cache, it reset all the content of the cache.
**/
template<class T,int ways,int rows>
void StaticAssoCache<T,ways,rows>::printStats(const char * name) const
{
	printf("%s cache hits: %lu, miss: %lu, ratio: %f\n",name,hits,miss,(float)hits/((float)hits+(float)miss));
}

}

#endif //NUMAPROF_STATIC_ASSO_CACHE_IMPL_HPP
