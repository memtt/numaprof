/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include "Stack.hpp"
#include "../common/Debug.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
/**
 * Compute the hash of the mini stack. Mostly by summing all the entries.
**/
void MiniStack::computeHash(void)
{
	size_t final = 0;
	for (int i = 0 ; i < NUMAPROG_MINI_STACk_SIZE ; i++)
		final += (size_t)stack[i];
	this->hash = final;
}

/*******************  FUNCTION  *********************/
/**
 * Compare two mini stracks to permit using a std:map.
**/
bool MiniStack::operator == (const MiniStack & node) const
{
	if (hash != node.hash)
		return false;
	for (int i = 0 ; i < NUMAPROG_MINI_STACk_SIZE ; i++)
		if (stack[i] != node.stack[i])
			return false;
	return true;
}

/*******************  FUNCTION  *********************/
/**
 * Less than operator to permit to put the MiniStack as index of a std::map.
**/
bool MiniStack::operator < (const MiniStack & node) const
{
	if (hash < node.hash)
	{
		return true;
	} else if (hash == node.hash) {
		for (int i = 0 ; i < NUMAPROG_MINI_STACk_SIZE ; i++)
		if (stack[i] >= node.stack[i])
			return false;
	return true;

	} else {
		return false;
	}
}

/*******************  FUNCTION  *********************/
/**
 * Strack constructor, do nothing 
**/
Stack::Stack(void)
{
}

/*******************  FUNCTION  *********************/
/**
 * Push a called child function.
 * @param value Address of the callee.
**/
void Stack::push(void * value)
{
	stack.push_back(value);
}

/*******************  FUNCTION  *********************/
/**
 * Pop the last callee;
**/
void Stack::pop(void)
{
	if (stack.size() == 0)
		numaprofWarning("Try to pop an empty stack !");
	stack.pop_back();
}

/*******************  FUNCTION  *********************/
/**
 * Fill the ministack with the last calls from the current stack.
 * @param miniStack Ministack to fill.
**/
void Stack::fillMiniStack(MiniStack & miniStack)
{
	//case
	size_t cnt = NUMAPROG_MINI_STACk_SIZE;
	if (stack.size() < NUMAPROG_MINI_STACk_SIZE)
		cnt = stack.size();
	
	//start
	size_t start = stack.size() - cnt;
	
	//copy
	for (size_t i = 0 ; i < cnt ; i++)
		miniStack.stack[i] = stack[start+i];
	
	//end
	for (size_t i = cnt ; i < NUMAPROG_MINI_STACk_SIZE ; i++)
		miniStack.stack[i] = NULL;
	
	//calc hash
	miniStack.computeHash();
}

/*******************  FUNCTION  *********************/
/**
 * Convert the mini stack to JSON format.
**/
void convertToJson(htopml::JsonState& json, const MiniStack& value)
{
	json.openArray();
	for (int i = 0 ; i < NUMAPROG_MINI_STACk_SIZE ; i++)
		if (value.stack[i] != NULL)
			json.printValue(value.stack[i]);
	json.closeArray();
}

}
