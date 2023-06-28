/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
#include <gtest/gtest.h>
#include <sstream>
#include "../core/Stack.hpp"
#include "../../extern-deps/from-htopml/json/ConvertToJson.h"

/***************** USING NAMESPACE ******************/
using namespace numaprof;

/*******************  FUNCTION  *********************/
TEST(Stack,constructor)
{
	Stack stack;
}

/*******************  FUNCTION  *********************/
TEST(Stack,push)
{
	Stack stack;
	stack.push((void*)0x1);
}

/*******************  FUNCTION  *********************/
TEST(Stack,pop)
{
	Stack stack;
	stack.push((void*)0x1);
	stack.pop();
}

/*******************  FUNCTION  *********************/
TEST(Stack,fillMiniStack_1)
{
	Stack stack;
	stack.push((void*)0x1);
	stack.push((void*)0x2);
	stack.push((void*)0x3);
	stack.push((void*)0x4);
	
	MiniStack mini;
	stack.fillMiniStack(mini);
	
	EXPECT_EQ((void*)0x2,mini.stack[0]);
	EXPECT_EQ((void*)0x3,mini.stack[1]);
	EXPECT_EQ((void*)0x4,mini.stack[2]);
	
	EXPECT_EQ(9ul,mini.hash);
}

/*******************  FUNCTION  *********************/
TEST(Stack,fillMiniStack_2)
{
	Stack stack;
	stack.push((void*)0x1);
	stack.push((void*)0x2);
	
	MiniStack mini;
	stack.fillMiniStack(mini);
	
	EXPECT_EQ((void*)0x1,mini.stack[0]);
	EXPECT_EQ((void*)0x2,mini.stack[1]);
	EXPECT_EQ((void*)0x0,mini.stack[2]);
	
	EXPECT_EQ(3ul,mini.hash);
}

/*******************  FUNCTION  *********************/
TEST(Stack,miniStackOp_1)
{
	Stack stack;
	stack.push((void*)0x1);
	stack.push((void*)0x2);
	stack.push((void*)0x3);
	
	MiniStack mini1;
	stack.fillMiniStack(mini1);
	
	stack.push((void*)0x4);
	
	MiniStack mini2;
	stack.fillMiniStack(mini2);
	
	EXPECT_TRUE(mini1 == mini1);
	EXPECT_FALSE(mini1 == mini2);
	
	EXPECT_TRUE(mini1 < mini2);
}

/*******************  FUNCTION  *********************/
TEST(Stack,mioniStackToJson)
{
	Stack stack;
	stack.push((void*)0x1);
	stack.push((void*)0x2);
	stack.push((void*)0x3);
	
	MiniStack mini1;
	stack.fillMiniStack(mini1);
	
	std::stringstream out;
	htopml::convertToJson(out,mini1);
	EXPECT_EQ("[\"0x1\", \"0x2\", \"0x3\"]",out.str());
}
