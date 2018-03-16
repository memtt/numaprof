/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.0-dev
             DATE     : 02/2018
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef NUMAPROF_STATIC_ASSO_CACHE_IMPL_HPP
#define NUMAPROF_STATIC_ASSO_CACHE_IMPL_HPP

/********************  HEADERS  *********************/
//std
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
		if (this->addr[r][w] == addr)
			return content[r][w];

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

}

#endif //NUMAPROF_STATIC_ASSO_CACHE_IMPL_HPP
