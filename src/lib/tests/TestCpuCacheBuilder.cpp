/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.0-dev
             DATE     : 02/2018
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
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

	CpuCacheBuilder::destroyLayout("dummy",layout);
	delete cache;
}
