/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef OPERATING_SYSTEM_HPP
#define OPERATING_SYSTEM_HPP

/*******************  HEADERS  **********************/
#include <cstdlib>
#include <string>

/*******************  NAMESPACE  ********************/
namespace numaprof
{

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
	static int getNumaOfPage(size_t addr);
	static int getNumaOfHugePage(size_t addr,bool * isHugePage);
};

}

#endif //OPERATING_SYSTEM_HPP
