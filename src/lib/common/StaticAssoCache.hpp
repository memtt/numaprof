/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.0-dev
             DATE     : 02/2018
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef NUMAPROF_STATIC_ASSO_CACHE_HPP
#define NUMAPROF_STATIC_ASSO_CACHE_HPP

/********************  HEADERS  *********************/
//std
#include <cstdlib>

/********************  HEADERS  *********************/

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  CLASS  **********************/
/**
 * Implement a cache to store datas identified by addresses. The cache is built
 * as an associative cache with N ways and M rows per way. It is built in template
 * to be faster and optimized by compiler.
**/
template<template class T,int ways,int rows>
class StaticAssoCache
{
	public:
		StaticAssoCache(void);
		T * get(size_t addr) const;
		void set(size_t addr,T * value);
	private:
		/** Store content **/
		T * content[rows][ways];
		/** store addresses identifying content to match requests **/
		size_t addr[rows][ways];
		/** Used to round robin on rows to override **/
		int next[rows];
}

/**********************  IMPL  **********************/
#include "StaticAssoCache_impl.hpp"

}

#endif //NUMAPROF_STATIC_ASSO_CACHE_HPP
