/*****************************************************
             PROJECT  : MATT
             VERSION  : 0.1.0-dev
             DATE     : 02/2015
             AUTHOR   : Valat Sébastien
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
//standard
#include <cstdio>
#include <cassert>
//MATT common
#include "Debug.hpp"
//locals
#include "LinuxProcMapReader.hpp"

/*******************  NAMESPACE  ********************/
namespace MATT
{

/*******************  FUNCTION  *********************/
void LinuxProcMapReader::load(void)
{
	//errors
	assert(procMap.empty());
	
	//open proc map
	FILE * fp = fopen("/proc/self/maps","r");
	assumeArg(fp != NULL,"Failed to read segment mapping in %1 : %2.").arg("/proc/self/map").argStrErrno().end();
	
	//loop on entries
	char buffer[4096];
	char ignored[4096];
	char fileName[4096];
	size_t ignored2;
	LinuxProcMapEntry entry;

	//loop on lines
	while (!feof(fp))
	{
		//load buffer
		char * res = fgets(buffer,sizeof(buffer),fp);
		
		//if ok, parse line
		if (res == buffer)
		{
			//parse
			int cnt = sscanf(buffer,"%p-%p %s %p %s %lu %s\n",&(entry.lower),&(entry.upper),ignored,&(entry.offset),ignored,&ignored2,fileName);
			
			//check args
			if (cnt == 7)
				entry.file = fileName;
			else if (cnt == 6)
				entry.file.clear();
			else
				MATT_FATAL_ARG("Invalid readline of proc/map entry : %1.").arg(buffer).end();
			
			//ok push
			procMap.push_back(entry);
		}
	}
	
	//close
	fclose(fp);
}

/*******************  FUNCTION  *********************/
const LinuxProcMapEntry * LinuxProcMapReader::getEntry(void* addr) const
{
	//search
	for (LinuxProcMap::const_iterator it = procMap.begin() ; it != procMap.end() ; ++it)
	{
		if ((size_t)it->lower <= (size_t)addr && (size_t)it->upper >= (size_t)addr)
			return &(*it);
	}
	
	//ok not found
	return NULL;
}

}
