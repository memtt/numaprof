/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../common/StaticAssoCache.hpp"

/***************** USING NAMESPACE ******************/
using namespace numaprof;

/*********************  TYPES  **********************/
typedef StaticAssoCache<size_t,4,32> TLB;

/*******************  FUNCTION  *********************/
TEST(StaticAssoCache,constructor)
{
	TLB tlb;
}

/*******************  FUNCTION  *********************/
TEST(StaticAssoCache,noHit)
{
	TLB tlb;
	size_t * out = tlb.get(1);
	EXPECT_EQ(NULL,out);
}


/*******************  FUNCTION  *********************/
TEST(StaticAssoCache,hit)
{
	TLB tlb;
	size_t ref;
	tlb.set(1,&ref);
	size_t * out = tlb.get(1);
	EXPECT_EQ(&ref,out);
}

/*******************  FUNCTION  *********************/
TEST(StaticAssoCache,checkMany)
{
	TLB tlb;
	size_t ref[8];
	
	for (int i = 0 ; i < 8 ; i++)
		tlb.set(i,&(ref[i]));
	
	for (int i = 0 ; i < 8 ; i++)
	{
		size_t * out = tlb.get(i);
		EXPECT_EQ(&ref[i],out);
	}
}

/*******************  FUNCTION  *********************/
TEST(StaticAssoCache,checkTooMany)
{
	TLB tlb;
	size_t ref[32*8];
	
	for (int i = 0 ; i < 32*8 ; i++)
		tlb.set(i,&(ref[i]));
	
	for (int i = 0 ; i < 32*4 ; i++)
	{
		size_t * out = tlb.get(i);
		EXPECT_EQ(NULL,out);
	}
	
	for (int i = 32*8 ; i < 32*8 ; i++)
	{
		size_t * out = tlb.get(i);
		EXPECT_EQ(&ref[i],out);
	}
}

