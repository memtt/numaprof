/*****************************************************
             PROJECT  : numaprof
             VERSION  : 0.0.0-dev
             DATE     : 05/2017
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
typedef std::vector<void*> StackStorage;

/*********************  CLASS  **********************/
struct MiniStack
{
	MiniStack(void) {for (int i = 0 ; i < NUMAPROG_MINI_STACk_SIZE ; i++) stack[i] = NULL; hash = 0;};
	void computeHash(void);
	bool operator == (const MiniStack & node) const;
	bool operator < (const MiniStack & node) const;
	void * stack[NUMAPROG_MINI_STACk_SIZE];
	size_t hash;
};

/*********************  CLASS  **********************/
class Stack
{
	public:
		Stack(void);
		void push(void * value);
		void pop(void);
		void fillMiniStack(MiniStack & miniStack);
	private:
		StackStorage stack;
};

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const MiniStack& value);

}

#endif //NUMAPROF_STACK_HPP
