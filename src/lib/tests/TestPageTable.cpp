/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../core/PageTable.hpp"

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

/*******************  FUNCTION  *********************/
TEST(PageTable,regAllocPointer_1)
{
	PageTable table;

	size_t addr = 2*1024*1024;
	table.regAllocPointer(addr,2ul*1024ul*1024ul,(void*)0x1);

	//page before
	Page & page1 = table.getPage(addr-1);
	EXPECT_EQ(PAGE_ALLOC_NONE,page1.allocStatus);
	EXPECT_EQ(NULL,page1.getAllocPointer(4095));

	//page after
	Page & page2 = table.getPage(addr+2*1024*1024);
	EXPECT_EQ(PAGE_ALLOC_NONE,page2.allocStatus);
	EXPECT_EQ(NULL,page2.getAllocPointer(0));

	//inside
	for (size_t i = 0 ; i < 2*1024*1024 ; i++)
	{
		Page & page = table.getPage(addr+i);
		if (i < 4096)
			ASSERT_EQ(PAGE_ALLOC_FRAG,page.allocStatus);
		else
			ASSERT_EQ(PAGE_ALLOC_FULL,page.allocStatus);
		ASSERT_EQ((void*)0x1,page.getAllocPointer(addr + i));
	}
}

/*******************  FUNCTION  *********************/
TEST(PageTable,regAllocPointer_2)
{
	PageTable table;

	size_t addr = 2*1024*1024+32;
	table.regAllocPointer(addr,2ul*1024ul*1024ul,(void*)0x1);

	//page before
	Page & page1 = table.getPage(addr-1);
	EXPECT_EQ(PAGE_ALLOC_FRAG,page1.allocStatus);
	EXPECT_EQ(NULL,page1.getAllocPointer(addr-1));

	//page after
	Page & page2 = table.getPage(addr+2*1024*1024);
	EXPECT_EQ(PAGE_ALLOC_FRAG,page2.allocStatus);
	EXPECT_EQ(NULL,page2.getAllocPointer(addr+2*1024*1024));

	//inside
	for (size_t i = 0 ; i < 2*1024*1024 ; i++)
	{
		Page & page = table.getPage(addr+i);
		if (i < 4096-32 || i >= 2*1024*1024-32)
			ASSERT_EQ(PAGE_ALLOC_FRAG,page.allocStatus);
		else
			ASSERT_EQ(PAGE_ALLOC_FULL,page.allocStatus);
		ASSERT_EQ((void*)0x1,page.getAllocPointer(addr + i));
	}
}

/*******************  FUNCTION  *********************/
TEST(PageTable,regAllocPointer_3)
{
	PageTable table;

	size_t addr = 2*1024*1024+64;
	table.regAllocPointer(addr,256,(void*)0x1);

	//page before
	Page & page1 = table.getPage(addr-1);
	EXPECT_EQ(PAGE_ALLOC_FRAG,page1.allocStatus);
	EXPECT_EQ(NULL,page1.getAllocPointer(addr-1));

	//page after
	Page & page2 = table.getPage(addr+256);
	EXPECT_EQ(PAGE_ALLOC_FRAG,page2.allocStatus);
	EXPECT_EQ(NULL,page2.getAllocPointer(addr+256));

	//inside
	for (size_t i = 0 ; i < 256 ; i++)
	{
		Page & page = table.getPage(addr+i);
		EXPECT_EQ(PAGE_ALLOC_FRAG,page.allocStatus);
		EXPECT_EQ((void*)0x1,page.getAllocPointer(addr + i));
	}
}

/*******************  FUNCTION  *********************/
TEST(PageTable,freePointer_1)
{
	PageTable table;

	size_t addr = 2*1024*1024;
	table.regAllocPointer(addr-32,32,(void*)0x1);
	table.regAllocPointer(addr,2ul*1024ul*1024ul,(void*)0x2);
	table.regAllocPointer(addr+2*1024*1024,32,(void*)0x3);

	//free middle one
	table.freeAllocPointer(addr,2*1024*1024,(void*)0x2);

	//page before
	Page & page1 = table.getPage(addr-1);
	EXPECT_EQ(PAGE_ALLOC_FRAG,page1.allocStatus);
	EXPECT_EQ((void*)0x1,page1.getAllocPointer(4095));

	//page after
	Page & page2 = table.getPage(addr+2*1024*1024);
	EXPECT_EQ(PAGE_ALLOC_FRAG,page2.allocStatus);
	EXPECT_EQ((void*)0x3,page2.getAllocPointer(0));

	//inside
	for (size_t i = 0 ; i < 2*1024*1024 ; i++)
	{
		Page & page = table.getPage(addr+i);
		if (i < 4096)
			ASSERT_EQ(PAGE_ALLOC_FRAG,page.allocStatus);
		else
			ASSERT_EQ(PAGE_ALLOC_NONE,page.allocStatus);
		ASSERT_EQ(NULL,page.getAllocPointer(addr + i));
	}
}

/*******************  FUNCTION  *********************/
TEST(PageTable,freePointer_2)
{
	PageTable table;

	size_t addr = 2*1024*1024+32;
	table.regAllocPointer(addr-32,32,(void*)0x1);
	table.regAllocPointer(addr,2ul*1024ul*1024ul,(void*)0x2);
	table.regAllocPointer(addr+2*1024*1024,32,(void*)0x3);

	//free middle one
	table.freeAllocPointer(addr,2*1024*1024,(void*)0x2);

	//page before
	Page & page1 = table.getPage(addr-1);
	EXPECT_EQ(PAGE_ALLOC_FRAG,page1.allocStatus);
	EXPECT_EQ((void*)0x1,page1.getAllocPointer(addr-1));

	//page after
	Page & page2 = table.getPage(addr+2*1024*1024);
	EXPECT_EQ(PAGE_ALLOC_FRAG,page2.allocStatus);
	EXPECT_EQ((void*)0x3,page2.getAllocPointer(addr+2*1024*1024));

	//inside
	for (size_t i = 0 ; i < 2*1024*1024 ; i++)
	{
		Page & page = table.getPage(addr+i);
		if (i < 4096-32 || i >= 2*1024*1024-32)
			ASSERT_EQ(PAGE_ALLOC_FRAG,page.allocStatus);
		else
			ASSERT_EQ(PAGE_ALLOC_NONE,page.allocStatus);
		ASSERT_EQ(NULL,page.getAllocPointer(addr + i));
	}
}

/*******************  FUNCTION  *********************/
TEST(PageTable,freePointer_3)
{
	PageTable table;

	size_t addr = 2*1024*1024+64;
	table.regAllocPointer(addr-32,32,(void*)0x1);
	table.regAllocPointer(addr,256,(void*)0x2);
	table.regAllocPointer(addr+256,32,(void*)0x3);

	//free middle one
	table.freeAllocPointer(addr,256,(void*)0x2);

	//page before
	Page & page1 = table.getPage(addr-1);
	EXPECT_EQ(PAGE_ALLOC_FRAG,page1.allocStatus);
	EXPECT_EQ((void*)0x1,page1.getAllocPointer(addr-1));

	//page after
	Page & page2 = table.getPage(addr+256);
	EXPECT_EQ(PAGE_ALLOC_FRAG,page2.allocStatus);
	EXPECT_EQ((void*)0x3,page2.getAllocPointer(addr+256));

	//inside
	for (size_t i = 0 ; i < 256 ; i++)
	{
		Page & page = table.getPage(addr+i);
		EXPECT_EQ(PAGE_ALLOC_FRAG,page.allocStatus);
		EXPECT_EQ(NULL,page.getAllocPointer(addr + i));
	}
}

/*******************  FUNCTION  *********************/
TEST(PageTable,clear)
{
	PageTable table;

	size_t addr = 2*1024*1024;
	table.regAllocPointer(addr-32,32,(void*)0x1);
	table.regAllocPointer(addr,2ul*1024ul*1024ul,(void*)0x2);
	table.regAllocPointer(addr+2*1024*1024,32,(void*)0x3);

	//free middle one
	table.clear(addr,2*1024*1024);

	//page before
	Page & page1 = table.getPage(addr-1);
	EXPECT_EQ(PAGE_ALLOC_FRAG,page1.allocStatus);
	EXPECT_EQ((void*)0x1,page1.getAllocPointer(4095));

	//page after
	Page & page2 = table.getPage(addr+2*1024*1024);
	EXPECT_EQ(PAGE_ALLOC_FRAG,page2.allocStatus);
	EXPECT_EQ((void*)0x3,page2.getAllocPointer(0));

	//inside
	for (size_t i = 0 ; i < 2*1024*1024 ; i++)
	{
		Page & page = table.getPage(addr+i);
		ASSERT_EQ(PAGE_ALLOC_NONE,page.allocStatus);
		ASSERT_EQ(NULL,page.getAllocPointer(addr + i));
	}
}
