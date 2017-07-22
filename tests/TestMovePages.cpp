/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../extern-deps/from-numactl/MovePages.hpp"
#include <cstring>

/***************** USING NAMESPACE ******************/
using namespace numaprof;

/*******************  FUNCTION  *********************/
TEST(MovePages,no_page)
{
	char * buffer = new char[20*1024*1024];
	EXPECT_EQ(-2,getNumaOfPage((size_t)buffer+10*1024*1024));
}


/*******************  FUNCTION  *********************/
TEST(MovePages,have_page)
{
	char * buffer = new char[20*1024*1024];
	memset(buffer,0,20*1024*1024);
	EXPECT_NE(-2,getNumaOfPage((size_t)buffer+10*1024*1024));
}