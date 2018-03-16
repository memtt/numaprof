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
template<template class T,int ways,int rows>
StaticAssoCache<T,ways,rows>::StaticAssoCache(void)
{
	for (int r = 0 ; r < rows ; r++)
	{
		next[r] = 0;
		for (int w = 0 ; w < ways ; w++)
		{
			content[r][w] = NULL;
			addr[r][w] = -1;
		}
	}
}

/*******************  FUNCTION  *********************/
/**
 * Look in the cache and return the corresponding address.
**/
template<template class T,int ways,int rows>
T * StaticAssoCache<T,ways,rows>::get(size_t addr)
{
	//compute row
	int r = addr % rows;
	
	//loop on ways
	for (int w = 0 ; w < ways ; w++)
		if (addr[r][w] == addr)
			return content[r][w];
}

/*******************  FUNCTION  *********************/
/**
 * Look in the cache and return the corresponding address.
**/
template<template class T,int ways,int rows>
T * StaticAssoCache<T,ways,rows>::set(size_t addr, T * value)
{
	//compute row
	int r = addr % rows;
	
	//loop on ways
	content[r][next[r]];
	
	//increment with round robin
	next[r] = (next[r]+1)%ways;
}

}

#endif //NUMAPROF_STATIC_ASSO_CACHE_IMPL_HPP
