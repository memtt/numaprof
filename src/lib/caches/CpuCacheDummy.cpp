/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include "CpuCacheDummy.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  FUNCTION  *********************/
/**
 * Default constructor
**/
CpuCacheDummy::CpuCacheDummy(void)
{
	//nothing to do
}

/*******************  FUNCTION  *********************/
bool CpuCacheDummy::onMemoryAccess(size_t addr)
{
	return false;
}

/*******************  FUNCTION  *********************/
void CpuCacheDummy::onThreadMove(const CpuBindList & cpuList)
{
	//nothing to do
}

}
