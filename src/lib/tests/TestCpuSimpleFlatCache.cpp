/*****************************************************
			 PROJECT  : numaprof
			 VERSION  : 1.1.0-dev
			 DATE     : 02/2018
			 AUTHOR   : Valat Sébastien
			 LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../caches/CpuSimpleFlatCache.hpp"

/***************** USING NAMESPACE ******************/
using namespace numaprof;

/*******************  CONSTS  **********************/
//TODO we test same than intel L1 cache 8 ways and 4K per way
#define CACHE_ASSO 8
#define CACHE_ROWS 4 //(4096/SIMPLE_FLAT_CACHE_LINE_SIZE)
#define CACHE_SIZE (CACHE_ROWS * CACHE_ASSO * SIMPLE_FLAT_CACHE_LINE_SIZE)

/*******************  FUNCTION  *********************/
TEST(CpuSimpleFlatCache,onMemoryAccessNoReuse_accessOnce)
{
	//base addr
	size_t base = 0xA0000;
	size_t size = CACHE_SIZE;
	
	//cache
	CpuSimpleFlatCache cache(size,CACHE_ASSO);

	//loop on whole memory once
	for (size_t offset = 0 ; offset < CACHE_SIZE * 4 ; offset += SIMPLE_FLAT_CACHE_LINE_SIZE) {
		ASSERT_FALSE(cache.onMemoryAccess(base+offset));
	}
}

/*******************  FUNCTION  *********************/
TEST(CpuSimpleFlatCache,onMemoryAccessNoReuse_doubleAccess)
{
	//base addr
	size_t base = 0xA0000;
	size_t size = CACHE_SIZE;
	
	//cache similar to intel L1 (64k)
	CpuSimpleFlatCache cache(size,CACHE_ASSO);

	//loop on whole memory once
	for (size_t offset = 0 ; offset < size * 4 ; offset += SIMPLE_FLAT_CACHE_LINE_SIZE) {
		ASSERT_FALSE(cache.onMemoryAccess(base+offset));
		ASSERT_TRUE(cache.onMemoryAccess(base+offset));
	}
}

/*******************  FUNCTION  *********************/
TEST(CpuSimpleFlatCache,onMemoryAccessNoReuse_sameRow)
{
	//base addr
	size_t base = 0xA0000;
	size_t size = CACHE_SIZE;
	
	//cache
	CpuSimpleFlatCache cache(size,CACHE_ASSO);

	//loop on whole memory once touching head of page for each
	for (size_t offset = 0 ; offset < size ; offset += size / CACHE_ASSO) {
		ASSERT_FALSE(cache.onMemoryAccess(base+offset));
		ASSERT_TRUE(cache.onMemoryAccess(base+offset));
	}

	//loop again to check
	for (size_t offset = 0 ; offset < size ; offset += size / CACHE_ASSO) {
		ASSERT_TRUE(cache.onMemoryAccess(base+offset));
	}

	//loop on more
	for (size_t offset = size ; offset < size * CACHE_ASSO ; offset += size / CACHE_ASSO) {
		ASSERT_FALSE(cache.onMemoryAccess(base+offset));
	}

	//loop on more again, should be evicted
	for (size_t offset = size ; offset < size * CACHE_SIZE ; offset += size / CACHE_ASSO) {
		ASSERT_FALSE(cache.onMemoryAccess(base+offset));
	}
}

/*******************  FUNCTION  *********************/
TEST(CpuSimpleFlatCache,onMemoryAccessNoReuse_loadOneWayThenAccessAgain)
{
	//base addr
	size_t base = 0xA0000;
	size_t size = CACHE_SIZE;
	
	//cache similar to intel L1 (64k)
	CpuSimpleFlatCache cache(size,8);

	//loop on whole memory once
	for (size_t offset = 0 ; offset < size / CACHE_ASSO ; offset += SIMPLE_FLAT_CACHE_LINE_SIZE) {
		ASSERT_FALSE(cache.onMemoryAccess(base+offset));
	}

	//loop again, this should be in cache
	for (size_t offset = 0 ; offset < size / CACHE_ASSO ; offset += SIMPLE_FLAT_CACHE_LINE_SIZE) {
		ASSERT_TRUE(cache.onMemoryAccess(base+offset));
	}
}

/*******************  FUNCTION  *********************/
TEST(CpuSimpleFlatCache,onMemoryAccessNoReuse_loadFullThenAccessAgain)
{
	//base addr
	size_t base = 0xA0000;
	size_t size = CACHE_SIZE;
	
	//cache similar to intel L1 (64k)
	CpuSimpleFlatCache cache(size,CACHE_ASSO);

	//loop on whole memory once
	for (size_t offset = 0 ; offset < size ; offset += SIMPLE_FLAT_CACHE_LINE_SIZE) {
		ASSERT_FALSE(cache.onMemoryAccess(base+offset));
	}

	//loop again, this should be in cache
	for (size_t offset = 0 ; offset < size ; offset += SIMPLE_FLAT_CACHE_LINE_SIZE) {
		ASSERT_TRUE(cache.onMemoryAccess(base+offset));
	}
}

/*******************  FUNCTION  *********************/
TEST(CpuSimpleFlatCache,onMemoryAccessNoReuse_loadFullThenAccessAgain_rescale)
{
	//base addr
	size_t base = 0xA0000;
	size_t size = CACHE_SIZE;
	
	//cache similar to intel L1 (64k)
	CpuSimpleFlatCache cache(size,CACHE_ASSO);

	//loop on whole memory once
	for (size_t offset = 0 ; offset < size ; offset += SIMPLE_FLAT_CACHE_LINE_SIZE) {
		ASSERT_FALSE(cache.onMemoryAccess(base+offset));
		for (int i = 0 ; i < 200 ; i++)
			ASSERT_TRUE(cache.onMemoryAccess(base+offset));
	}

	//loop again, this should be in cache
	for (size_t offset = 0 ; offset < size ; offset += SIMPLE_FLAT_CACHE_LINE_SIZE) {
		ASSERT_TRUE(cache.onMemoryAccess(base+offset));
	}
}