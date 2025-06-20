/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.6
             DATE     : 06/2025
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef OPERATING_SYSTEM_HPP
#define OPERATING_SYSTEM_HPP

/*******************  HEADERS  **********************/
#include <cstdlib>
#include <string>
#include <cstdint>

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/*******************  DEFINE  ***********************/
#define MALT_NUMA_UNBOUND -1

/*********************  STRUCT  *********************/
struct PageMapEntry
{
	size_t pageFrameNumber:55;
	uint8_t softDirty:1;
	uint8_t exclusivelyMapped:1;
	uint8_t uffdWriteProtected:1;
	uint8_t guardRegioin:1;
	uint8_t ignored:2;
	uint8_t isFilePageOrSharedAnon:1;
	uint8_t swaped:1;
	uint8_t present:1;
};

/*******************  FUNCTION  *********************/
struct OS
{
	static char * loadTxtFile(const char * path,size_t maxSize = 4096);
	static std::string getExeName(void);
	static std::string getCmdLine(void);
	static std::string getHostname(void);
	static std::string getDateTime(void);
	static int getPID(void);
	static int getTID(void);
	static int getNumaOfPage(size_t addr, int numaOfThread);
	static int getNumaOfHugePage(size_t addr,bool * isHugePage, int numaOfThread);
	static bool getPageMapPresent(size_t addr);
	static int emulateNumaNode(int numaOfThread);
};

}

#endif //OPERATING_SYSTEM_HPP
