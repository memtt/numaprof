/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../caches/CpuCacheDummy.hpp"

/***************** USING NAMESPACE ******************/
using namespace numaprof;

/*******************  FUNCTION  *********************/
TEST(CpuCacheDummy,onMemoryAccess)
{
	CpuCacheDummy cache;
    EXPECT_FALSE(cache.onMemoryAccess(0xA32));
}
