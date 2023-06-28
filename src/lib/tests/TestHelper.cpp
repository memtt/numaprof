/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include "../common/Helper.hpp"

/***************** USING NAMESPACE ******************/
using namespace numaprof;

/*******************  FUNCTION  *********************/
TEST(Helper,extractNth)
{
	const char lst[] = "0-10,15-20,30-40";

    char buf[32];
    bool status = Helper::extractNth(buf,lst,',',0);
    EXPECT_TRUE(status);
    EXPECT_EQ("0-10",std::string(buf));

    status = Helper::extractNth(buf,lst,',',1);
    EXPECT_TRUE(status);
    EXPECT_EQ("15-20",std::string(buf));

    status = Helper::extractNth(buf,lst,',',2);
    EXPECT_TRUE(status);
    EXPECT_EQ("30-40",std::string(buf));

    status = Helper::extractNth(buf,lst,',',3);
    EXPECT_FALSE(status);
}

/*******************  FUNCTION  *********************/
TEST(Range,constructor_1)
{
    Range range("10-20");
    EXPECT_EQ(10,range.start);
    EXPECT_EQ(20,range.end);
}

/*******************  FUNCTION  *********************/
TEST(Range,constructor_2)
{
    Range range("10");
    EXPECT_EQ(10,range.start);
    EXPECT_EQ(10,range.end);
}

/*******************  FUNCTION  *********************/
TEST(Range,contain)
{
    Range range("10-20");
    EXPECT_FALSE(range.contain(9));
    EXPECT_TRUE(range.contain(10));
    EXPECT_TRUE(range.contain(15));
    EXPECT_TRUE(range.contain(20));
    EXPECT_FALSE(range.contain(21));
}
