/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef NUMAPROF_STACK_HPP
#define NUMAPROF_STACK_HPP

/*******************  HEADERS  **********************/
#include <cstdlib>
#include <vector>
#include "../../../extern-deps/from-htopml/json/ConvertToJson.h"

/********************  MACRO  ***********************/
#define NUMAPROG_MINI_STACk_SIZE 3

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  TYPES  **********************/
/**
 * Vector to store the stack pointers.
**/
typedef std::vector<void*> StackStorage;

/*********************  CLASS  **********************/
/** 
 * Ministrack of a fixed size used in the thread and process handler
 * to identify the instructions.
**/
struct MiniStack
{
	MiniStack(void) {for (int i = 0 ; i < NUMAPROG_MINI_STACk_SIZE ; i++) stack[i] = NULL; hash = 0;};
	void computeHash(void);
	bool operator == (const MiniStack & node) const;
	bool operator < (const MiniStack & node) const;
	/** Store the N last element of the real stack, with caller first. **/
	void * stack[NUMAPROG_MINI_STACk_SIZE];
	/** hash of the stack to quicly access and construct the parent hash map **/
	size_t hash;
};

/*********************  CLASS  **********************/
/**
 * Keep track of the call stacks.
**/
class Stack
{
	public:
		Stack(void);
		void push(void * value);
		void pop(void);
		void fillMiniStack(MiniStack & miniStack);
	private:
		/** Store the stack pointer with caller first **/
		StackStorage stack;
};

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const MiniStack& value);

}

#endif //NUMAPROF_STACK_HPP
