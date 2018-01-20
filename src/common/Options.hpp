/*****************************************************
             PROJECT  : numaprof
             VERSION  : 0.0.0-dev
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef NUMAPROF_OPTIONS_HPP
#define NUMAPROF_OPTIONS_HPP

/********************  HEADERS  *********************/
//std
#include <string>
#include <cassert>

/********************  HEADERS  *********************/
//iniparser
extern "C" {
#include "../../extern-deps/iniparser/src/iniparser.h"
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
	bool operator==(const Options & value) const;
	//output
	std::string outputName;
	bool outputIndent;
	bool outputJson;
	bool outputDumpConfig;
	bool outputSilent;
	bool outputRemoveSmall;
	float outputRemoveRatio;
	//core
	bool coreSkipStackAccesses;
	int coreThreadCacheEntries;
	//info
	bool infoHidden;
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
Options & initGlobalOptions(void);

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
