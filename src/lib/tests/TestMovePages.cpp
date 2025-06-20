/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.6
             DATE     : 06/2025
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../common/Options.hpp"
#include "../portability/OS.hpp"
#include <cstring>

/***************** USING NAMESPACE ******************/
using namespace numaprof;

/*******************  FUNCTION  *********************/
TEST(MovePages,no_page)
{
	char * buffer = new char[20*1024*1024];
	initGlobalOptions(true);
	EXPECT_LT(OS::getNumaOfPage((size_t)buffer+10*1024*1024,MALT_NUMA_UNBOUND), 0);
}


/*******************  FUNCTION  *********************/
TEST(MovePages,have_page)
{
	char * buffer = new char[20*1024*1024];
	memset(buffer,0,20*1024*1024);
	initGlobalOptions(true);
	EXPECT_GE(OS::getNumaOfPage((size_t)buffer+10*1024*1024,MALT_NUMA_UNBOUND), 0);
}

/*******************  FUNCTION  *********************/
TEST(MovePages,have_page_emulate_numa)
{
	char * buffer = new char[20*1024*1024];
	memset(buffer,0,20*1024*1024);
	initGlobalOptions(true);
	gblOptions->emulateNuma = 4;
	EXPECT_GE(OS::getNumaOfPage((size_t)buffer+10*1024*1024,MALT_NUMA_UNBOUND), 0);
}

/*******************  FUNCTION  *********************/
TEST(MovePages,have_page_emulate_numa_bound)
{
	char * buffer = new char[20*1024*1024];
	memset(buffer,0,20*1024*1024);
	initGlobalOptions(true);
	gblOptions->emulateNuma = 4;
	EXPECT_EQ(OS::getNumaOfPage((size_t)buffer+10*1024*1024,0), 0);
}