/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  HEADERS  **********************/
#include "PageTable.hpp"
#include "Stats.hpp"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  STRUCT  ***********************/
struct MallocInfos
{
	size_t ptr;
	size_t size;
	Stats * stats;
};

/*******************  FUNCTION  *********************/
class MallocTracker
{
	public:
		MallocTracker(PageTable * pageTable);
		void onAlloc(size_t ip,size_t ptr, size_t size);
		void onFree(size_t ptr);
		void flush(class ProcessTracker * process);
	private:
		PageTable * pageTable;
		InstrInfoMap instructions;
};

}
