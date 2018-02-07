/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.0-dev
             DATE     : 02/2018
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../portability/OS.hpp"
#include <cstring>

/***************** USING NAMESPACE ******************/
using namespace numaprof;

/*******************  FUNCTION  *********************/
TEST(MovePages,no_page)
{
	char * buffer = new char[20*1024*1024];
	EXPECT_EQ(-2,OS::getNumaOfPage((size_t)buffer+10*1024*1024));
}


/*******************  FUNCTION  *********************/
TEST(MovePages,have_page)
{
	char * buffer = new char[20*1024*1024];
	memset(buffer,0,20*1024*1024);
	EXPECT_NE(-2,OS::getNumaOfPage((size_t)buffer+10*1024*1024));
}
