/*****************************************************
             PROJECT  : numaprof
             VERSION  : 0.0.0-dev
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef ACCESS_MATRIX_HPP
#define ACCESS_MATRIX_HPP

/*******************  HEADERS  **********************/
#include <cassert>
#include "../../extern-deps/from-htopml/json/ConvertToJson.h"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*********************  STRUCT  *********************/
class AccessMatrix
{
	public:
		AccessMatrix(int numaNodes);
		~AccessMatrix(void);
		void merge(AccessMatrix & value);
		void access(int threadNumaNode,int pageNumaNode) {assert(threadNumaNode >= -1 && threadNumaNode < numaNodes); assert(pageNumaNode >= 0 && pageNumaNode < numaNodes); matrix[threadNumaNode+1][pageNumaNode]++;};
		friend void convertToJson(htopml::JsonState& json, const AccessMatrix& value);
	private:
		size_t ** matrix;
		int numaNodes;
};

}

#endif //ACCESS_MATRIX_HPP
