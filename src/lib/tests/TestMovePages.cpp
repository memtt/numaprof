/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
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
	EXPECT_LT(OS::getNumaOfPage((size_t)buffer+10*1024*1024), 0);
}


/*******************  FUNCTION  *********************/
TEST(MovePages,have_page)
{
	char * buffer = new char[20*1024*1024];
	memset(buffer,0,20*1024*1024);
	EXPECT_GE(OS::getNumaOfPage((size_t)buffer+10*1024*1024), 0);
}
