/*****************************************************
			 PROJECT  : numaprof
			 VERSION  : 1.1.0-dev
			 DATE     : 02/2018
			 AUTHOR   : Valat Sébastien
			 LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../common/Options.hpp"
#include "../caches/CpuCacheBuilder.hpp"

/***************** USING NAMESPACE ******************/
using namespace numaprof;

/*******************  FUNCTION  *********************/
TEST(CpuCacheBuilder,buildDummy)
{
	void * layout = CpuCacheBuilder::buildLayout("dummy");
	CpuCache * cache = CpuCacheBuilder::buildCache("dummy",layout);

	EXPECT_EQ(NULL,layout);
	EXPECT_FALSE(cache->onMemoryAccess(0xA32));

	delete cache;
	CpuCacheBuilder::destroyLayout("dummy",layout);
}

/*******************  FUNCTION  *********************/
TEST(CpuCacheBuilder,buildL1)
{
	initGlobalOptions(true);

	void * layout = CpuCacheBuilder::buildLayout("L1");
	CpuCache * cache = CpuCacheBuilder::buildCache("L1",layout);

	EXPECT_EQ(NULL,layout);
	EXPECT_FALSE(cache->onMemoryAccess(0xA32));

	delete cache;
	CpuCacheBuilder::destroyLayout("L1",layout);
}

/*******************  FUNCTION  *********************/
TEST(CpuCacheBuilder,buildL1Satic)
{
	initGlobalOptions(true);

	void * layout = CpuCacheBuilder::buildLayout("L1_static");
	CpuCache * cache = CpuCacheBuilder::buildCache("L1_static",layout);

	EXPECT_EQ(NULL,layout);
	EXPECT_FALSE(cache->onMemoryAccess(0xA32));

	delete cache;
	CpuCacheBuilder::destroyLayout("L1_static",layout);
}

