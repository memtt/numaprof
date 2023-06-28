/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef NUMAPROF_OPTIONS_HPP
#define NUMAPROF_OPTIONS_HPP

/********************  HEADERS  *********************/
//std
#include <string>
#include <cassert>
#include <vector>

/********************  HEADERS  *********************/
//iniparser
extern "C" {
#include "../../../extern-deps/iniparser/src/iniparser.h"
}

/*********************  TYPES  **********************/
namespace htopml {
	class JsonState;
}

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/********************  STRUCT  **********************/
/**
 * Structure to manage the NUMAPROF options. It manage loading status from a config 
 * file in INI format.
 * 
 * @brief Structure to manage all the NUMAPROF options.
**/
struct Options
{
	Options(void);
	void loadFromFile(const char * fname);
	void loadFromString(const char * value);
	void loadFromIniDic(dictionary * iniDic);
	void dumpConfig(const char * fname) const;
	void cache(void);
	bool operator==(const Options & value) const;
	//output
	/**
	 * Format used to save all the output files. You can use some variables :
	 *  - %1 : will be replaced by executable name 
	 *  - %2 : will be replaced by PID
	 *  - %3 : will be replaced by file extension.
	**/
	std::string outputName;
	/** Enable indentation in the JSON or LUA format **/
	bool outputIndent;
	/** Enable JSON output format **/
	bool outputJson;
	/** Dump the current config into a ini file at the end of the profiling **/
	bool outputDumpConfig;
	/** 
	 * Disable NUMAPROF outputs, usefull if you instrument a program from 
	 * which you use the output 
	**/
	bool outputSilent;
	/** 
	 * Remove all small values before dumping the profile file. This might lead to
	 * a large reduction of the file size. The cutoff limit will be outputRemoveRatio.
	**/
	bool outputRemoveSmall;
	/** Define the cutoff for outputRemoveSmall **/
	float outputRemoveRatio;
	//core
	/** 
	 * If true (default), do not instrument the accesses to the local stack.
	 * This provide a speed-up of a factor roughtly 2.
	**/
	bool coreSkipStackAccesses;
	/**
	 * Size of the buffer to cache the per thread metrics before synchronizing
	 * them with the global process one. This increase scalability by limiting
	 * locks and atomic operations.
	**/
	int coreThreadCacheEntries;
	/**
	 * Define if the binary objects (code) is considered pinned or not pinned by default.
	 * This is to be used for data from executable and so files loaded in memory without a first 
	 * touch access.
	**/
	bool coreObjectCodePinned;
	/**
	 * Configure the access batch size
	**/
	int coreAccessBatchSize;
	/**
	 * Do not track memory accesses of the given memory. Use semi-column to separate.
	 * You provide a string which is search so you are not forced to use the full path or
	 * full name.
	**/
	std::string coreSkipBinaries;
	/**
	 * Conversion of coreSkipBinaries in a list to be efficiently used. It has
	 * no relation with the direct entries in the config file.
	**/
	std::vector<std::string> coreSkipBinariesVect;
	/** 
	 * Define the type of cache to simulate.
	**/
	std::string cacheType;
	//info
	/** Hide paths and machine name if you don't want critical informations to be leaked. **/
	bool infoHidden;
	/** Define cache size **/
	std::string cacheSize;
	/** Define cache assoiativity **/
	int cacheAssociativity;
	/** Use the MPI rank instead of PID */
	bool mpiUseRank;
	/** Rank is detected by env var, use 'auto' or give a variable name. */
	std::string mpiRankVar;
};

/********************  GLOBALS  *********************/
/** 
 * Define a global instance of option to get access from the whole tool. 
 * Please prefer to use the accessor instead of the variable itsef directly.
 * The code will be inlined by be safer in debug mode.
**/
extern Options * gblOptions;

/*******************  FUNCTION  *********************/
static inline const Options & getGlobalOptions(void)
{
	assert(gblOptions != NULL);
	return *gblOptions;
}

/*******************  FUNCTION  *********************/
Options & initGlobalOptions(bool reinint = false);

/*******************  FUNCTION  *********************/
/** Safer function to access to the option, with check in debug mode.**/
static inline Options & getOptions(void) 
{
	assert(gblOptions != NULL);
	return *gblOptions;
}

/*******************  FUNCTION  *********************/
/*
 * Provide some helper functions to use INI parser.
**/
struct IniParserHelper
{
	static std::string extractSectionName ( const char * key );
	static void setEntry (dictionary * dic, const char * key, const char* value );
	static void setEntry (dictionary * dic, const char * key, bool value);
	static void setEntry (dictionary * dic, const char * key, int value);
	static void setEntry (dictionary * dic, const char * key, float value);
};

/*******************  FUNCTION  *********************/
void convertToJson(htopml::JsonState & json,const Options & value);

}

#endif //NUMAPROF_OPTIONS_HPP
