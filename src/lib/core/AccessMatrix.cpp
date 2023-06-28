/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
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
/**
 * Constructor of the access matrix, it mostly allocate the matrix
 * memory.
 * @param numaNodes Number of numa nodes to consider for memory allocation
 * and access checking.
**/
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
/**
 * Destructor of the matrix. Free the memory.
**/
AccessMatrix::~AccessMatrix(void)
{
	for (int i = 0 ; i < numaNodes+1 ; i++)
		delete [] this->matrix[i];
	delete [] this->matrix;
	this->matrix = NULL;
}

/*******************  FUNCTION  *********************/
/**
 * Merge two access matrixes, mostly used to get the process access matrix
 * from all the per thread access matrix.
 * @param value Define the matrix to merge on the current one.
**/
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
/**
 * Consvert the access matrix into json format.
 * @param json Define the json obect to use for the conversion.
 * @param value Define the access matrix to convert.
**/
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
