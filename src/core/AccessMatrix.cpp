/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/


/*******************  HEADERS  **********************/
#include <cstdio>
#include "AccessMatrix.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
AccessMatrix::AccessMatrix(int numaNodes)
{
	assert(numaNodes > 0);
	this->numaNodes = numaNodes;
	this->matrix = new size_t*[numaNodes+1];
	for (int i = 0 ; i < numaNodes+1 ; i++)
	{
		this->matrix[i] = new size_t[numaNodes];
		for (int j = 0 ; j < numaNodes ; j++)
			this->matrix[i][j] = 0;
	}
}

/*******************  FUNCTION  *********************/
AccessMatrix::~AccessMatrix(void)
{
	for (int i = 0 ; i < numaNodes+1 ; i++)
		delete [] this->matrix[i];
	delete [] this->matrix;
	this->matrix = NULL;
}

/*******************  FUNCTION  *********************/
void AccessMatrix::merge(AccessMatrix & value)
{
	//check
	assert(numaNodes == value.numaNodes);
	
	//run
	for (int i = 0 ; i < numaNodes+1 ; i++)
		for (int j = 0 ; j < numaNodes ; j++)
			this->matrix[i][j] += value.matrix[i][j];
}

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState& json, const AccessMatrix& value)
{
	json.openStruct();
		for (int i = -1 ; i < value.numaNodes ; i++)
		{
			char node[128];
			sprintf(node,"%d",i);
			json.openFieldArray(node);
				for (int j = 0 ; j < value.numaNodes ; j++)
					json.printValue(value.matrix[i+1][j]);
			json.closeFieldArray(node);
		}
	json.closeStruct();
}

}
