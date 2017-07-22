/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../src/PageTable.hpp"

/***************** USING NAMESPACE ******************/
using namespace numaprof;

/*******************  FUNCTION  *********************/
TEST(PageTable,constructor)
{
	PageTable table;
}

/*******************  FUNCTION  *********************/
TEST(PageTable,setup)
{
	PageTable table;
	
	void * ptr = malloc(16);
	Page & page = table.getPage((size_t)ptr);
	EXPECT_EQ(NUMAPROF_DEFAULT_NUMA_NODE,page.numaNode);
	
	//set
	table.getPage((size_t)ptr).numaNode = 1;
	table.getPage((size_t)ptr+4096).numaNode = 2;
	
	//check
	EXPECT_EQ(1,table.getPage((size_t)ptr).numaNode);
	EXPECT_EQ(2,table.getPage((size_t)ptr+4096).numaNode);
	
	free(ptr);
}

