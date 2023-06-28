/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstdio>
#include <cassert>
//internals
#include "Debug.hpp"
#include "Options.hpp"
#include "Helper.hpp"
#include "../../../extern-deps/from-htopml/json/ConvertToJson.h"

/*******************  NAMESPACE  ********************/
namespace numaprof 
{

/********************  GLOBALS  *********************/
Options * gblOptions = NULL;

/*******************  FUNCTION  *********************/
/**
 * Constructor to setup the default values for each options
**/
Options::Options(void)
{
	//output
	this->outputName              = "numaprof-%1-%2.%3";
	this->outputIndent            = false;
	this->outputJson              = true;
	this->outputDumpConfig        = false;
	this->outputSilent            = true;
	this->outputRemoveSmall       = false;
	this->outputRemoveRatio       = 0.5;
	//core
	this->coreSkipStackAccesses   = true;
	this->coreThreadCacheEntries     = 512;
	this->coreObjectCodePinned    = false;
	this->coreSkipBinaries        = "";
	this->coreAccessBatchSize     = 0;
	//info
	this->infoHidden              = false;
	//cache
	this->cacheType               = "dummy";
	this->cacheSize               = "32K";
	this->cacheAssociativity      = 8;
	//mpi
	this->mpiUseRank              = false;
	this->mpiRankVar              = "auto";
}

/*******************  FUNCTION  *********************/
/**
 * Manage operator == to help validation in unit test suite.
**/
bool Options::operator==(const Options& value) const
{
	//output
	if (this->outputName != value.outputName) return false;
	if (this->outputIndent != value.outputIndent) return false;
	if (this->outputJson != value.outputJson) return false;
	if (this->outputDumpConfig != value.outputDumpConfig) return false;
	if (this->outputSilent != value.outputSilent)  return false;
	if (this->outputRemoveSmall != value.outputRemoveSmall) return false;
	if (this->outputRemoveRatio != value.outputRemoveRatio) return false;
	//core
	if (this->coreSkipStackAccesses != value.coreSkipStackAccesses) return false;
	if (this->coreThreadCacheEntries != value.coreThreadCacheEntries) return false;
	if (this->coreObjectCodePinned != value.coreObjectCodePinned) return false;
	if (this->coreSkipBinaries != value.coreSkipBinaries) return false;
	if (this->coreAccessBatchSize != value.coreAccessBatchSize) return false;
	//info
	if (this->infoHidden != value.infoHidden) return false;
	//cache
	if (this->cacheType != value.cacheType) return false;
	if (this->cacheSize != value.cacheSize) return false;
	if (this->cacheAssociativity != value.cacheAssociativity) return false;
	//mpi
	if (this->mpiUseRank != value.mpiUseRank) return false;
	if (this->mpiRankVar != value.mpiRankVar) return false;
	
	return true;
}

/*******************  FUNCTION  *********************/
/**
 * Load values from string, mostly to be used from MALT_OPTION environment variable.
 * 
 * It expect string format like :
 * 
 * SEC1:NAME1=VALUE1;SEC2:NAME2=VALUE2;
 * 
 * @param value Define the string to load as a config file.
**/
void Options::loadFromString ( const char* value )
{
	//trivial
	if (value == NULL)
		return;

	//create fake dictionary
	dictionary * dic = dictionary_new(10);
	
	//copy string
	char * dump = strdup(value);
	
	//loop on separators ','
	char * cur = dump;
	while (*cur != '\0')
	{
		//remind start
		char * start = cur;
		char * sep = NULL;
		
		//search ',' or '\0'
		while (*cur != ',' && *cur != '\0')
		{
			if (*cur == '=')
				sep = cur;
			cur++;
		}
		
		//skip to next
		if (cur == start)
		{
			cur++;
			continue;
		}
		
		//is end
		bool isEnd = (*cur == '\0');
		char buffer[256];
		sprintf(buffer,"Invalid string format to setup option : '%s', expect SECTION:NAME=VALUE.",start);
		numaprofAssume(sep != NULL,buffer);
		
		//cut strings
		*cur = '\0';
		*sep = '\0';
		sep++;
		
		//setup in INI
		IniParserHelper::setEntry(dic,start,sep);
		
		//move
		if (isEnd == false)
			cur++;
	}
	
	//load
	this->loadFromIniDic(dic);

	//free
	iniparser_freedict(dic);
	free(dump);
}

/*******************  FUNCTION  *********************/
/**
 * Internal function to load options from iniDic.
**/
void Options::loadFromIniDic ( dictionary* iniDic )
{
	//errors
	assert(iniDic != NULL);
	
	//load values for output
	this->outputName          = iniparser_getstring(iniDic,"output:name",(char*)this->outputName.c_str());
	this->outputIndent        = iniparser_getboolean(iniDic,"output:indent",this->outputIndent);
	this->outputJson          = iniparser_getboolean(iniDic,"output:json",this->outputJson);
	this->outputDumpConfig    = iniparser_getboolean(iniDic,"output:config",this->outputDumpConfig);
	this->outputSilent        = iniparser_getboolean(iniDic,"output:silent",this->outputSilent);
	this->outputRemoveSmall   = iniparser_getboolean(iniDic,"output:removeSmall",this->outputRemoveSmall);
	this->outputRemoveRatio   = iniparser_getdouble(iniDic,"output:removeRatio",this->outputRemoveRatio);
	
	//core
	this->coreSkipStackAccesses = iniparser_getboolean(iniDic,"core:skipStackAccesses",this->coreSkipStackAccesses);
	this->coreThreadCacheEntries = iniparser_getint(iniDic,"core:threadCacheEntries",this->coreThreadCacheEntries);
	this->coreObjectCodePinned = iniparser_getint(iniDic,"core:objectCodePinned",this->coreObjectCodePinned);
	this->coreSkipBinaries = iniparser_getstring(iniDic,"core:skipBinaries",(char*)this->coreSkipBinaries.c_str());
	this->coreAccessBatchSize = iniparser_getint(iniDic,"core:accessBatchSize",this->coreAccessBatchSize);
	
	//info
	this->infoHidden          = iniparser_getboolean(iniDic,"info:hidden",this->infoHidden);

	//cache
	this->cacheType           = iniparser_getstring(iniDic,"cache:type",(char*)this->cacheType.c_str());
	this->cacheSize           = iniparser_getstring(iniDic,"cache:size",(char*)this->cacheSize.c_str());
	this->cacheAssociativity  = iniparser_getint(iniDic,"cache:associativity",this->cacheAssociativity);

	//mpi
	this->mpiUseRank          = iniparser_getboolean(iniDic, "mpi:useRank", this->mpiUseRank);
	this->mpiRankVar          = iniparser_getstring(iniDic, "mpi:rankVar", (char*)this->mpiRankVar.c_str());
}

/*******************  FUNCTION  *********************/
void Options::cache ( void )
{
	bool status;
	char buffer[4096];
	int i = 0;
	while((status = Helper::extractNth(buffer,coreSkipBinaries.c_str(),';',i++)))
		coreSkipBinariesVect.push_back(buffer);
}

/*******************  FUNCTION  *********************/
/**
 * Function to load options from a config file in INI format.
**/
void Options::loadFromFile(const char* fname)
{
	//load ini dic
	dictionary * iniDic;
	assert(fname != NULL);
	iniDic = iniparser_load(fname);
	
	//errors
	char buffer[128];
	sprintf(buffer,"Failed to load config file : %s !",fname);
	numaprofAssume(iniDic != NULL,buffer);
	
	//load
	loadFromIniDic(iniDic);
	
	//free dic
	iniparser_freedict(iniDic);
	
	//TODO apply getenv MALT_OPTIONS to override here and add "envOverride" parameter to enable it from caller
}

/*******************  FUNCTION  *********************/
/**
 * Helper function to convert the options to JSON output format and dump it
 * into the MALT output profile.
**/
void convertToJson(htopml::JsonState & json,const Options & value)
{
	json.openStruct();
		json.openFieldStruct("output");
			json.printField("dumpConfig",value.outputDumpConfig);
			json.printField("index",value.outputIndent);
			json.printField("json",value.outputJson);
			json.printField("name",value.outputName);
			json.printField("silent",value.outputSilent);
			json.printField("removeSmall",value.outputRemoveSmall);
			json.printField("removeRatio",value.outputRemoveRatio);
		json.closeFieldStruct("output");
		
		json.openFieldStruct("core");
			json.printField("skipStackAccesses",value.coreSkipStackAccesses);
			json.printField("threadCacheEntries",value.coreThreadCacheEntries);
			json.printField("ojectCodePinned",value.coreObjectCodePinned);
			json.printField("skipBinaries",value.coreSkipBinaries);
			json.printField("accessBatchSize",value.coreAccessBatchSize);
		json.closeFieldStruct("core");
		
		json.openFieldStruct("info");
			json.printField("hidden",value.infoHidden);
		json.closeFieldStruct("info");

		json.openFieldStruct("cache");
			json.printField("type",value.cacheType);
			json.printField("size",value.cacheSize);
			json.printField("associativity",value.cacheAssociativity);
		json.closeFieldStruct("cache");

		json.openFieldStruct("mpi");
			json.printField("useRank", value.mpiUseRank);
			json.printField("rankVar", value.mpiRankVar);
		json.closeFieldStruct("mpi");
	json.closeStruct();
}

/*******************  FUNCTION  *********************/
/**
 * Helper to dump the config as INI file.
**/
void Options::dumpConfig(const char* fname) const
{
	//create dic
	assert(fname != NULL);
	dictionary * dic = dictionary_new(10);
	
	//output
	IniParserHelper::setEntry(dic,"output:name",this->outputName.c_str());
	IniParserHelper::setEntry(dic,"output:json",this->outputJson);
	IniParserHelper::setEntry(dic,"output:indent",this->outputIndent);
	IniParserHelper::setEntry(dic,"output:config",this->outputDumpConfig);
	IniParserHelper::setEntry(dic,"output:silent",this->outputSilent);
	IniParserHelper::setEntry(dic,"output:removeSmall",this->outputRemoveSmall);
	IniParserHelper::setEntry(dic,"output:removeRatio",this->outputRemoveRatio);
	
	//core
	IniParserHelper::setEntry(dic,"core:skipStackAccesses",this->coreSkipStackAccesses);
	IniParserHelper::setEntry(dic,"core:threadCacheEntries",this->coreThreadCacheEntries);
	IniParserHelper::setEntry(dic,"core:objectCodePinned",this->coreObjectCodePinned);
	IniParserHelper::setEntry(dic,"core:skipFiled",this->coreSkipBinaries.c_str());
	IniParserHelper::setEntry(dic,"core:accessBatchSize",this->coreAccessBatchSize);
	
	//info
	IniParserHelper::setEntry(dic,"info:hidden",this->infoHidden);

	//cache
	IniParserHelper::setEntry(dic,"cache:type",this->cacheType.c_str());
	IniParserHelper::setEntry(dic,"cache:size",this->cacheSize.c_str());
	IniParserHelper::setEntry(dic,"cache:associativity",this->cacheAssociativity);

	//mpi
	IniParserHelper::setEntry(dic,"mpi:usrRank",this->mpiUseRank);
	IniParserHelper::setEntry(dic,"mpi:rankVar",this->mpiRankVar.c_str());

	//write
	FILE * fp = fopen(fname,"w");
	char buffer[2048];
	sprintf(buffer,"Failed to write dump of config file into %s : %s !",fname,strerror(errno));
	numaprofAssume(fp != NULL,buffer);
	iniparser_dump_ini(dic,fp);
	fclose(fp);
	
	//free
	iniparser_freedict(dic);
}

/*******************  FUNCTION  *********************/
/**
 * Internal function to split strings on ':' and extract the section name.
**/
std::string IniParserHelper::extractSectionName ( const char * key )
{
	std::string tmp;
	int i = 0;
	while (key[i] != ':' && key[i] != '\0')
		tmp += key[i++];
	return tmp;
}

/*******************  FUNCTION  *********************/
/**
 * Updat some entries of a dictionnary.
 * @param dic Define the dictionnary to update.
 * @param key Define the key to update.
 * @param value Define the value to setup for the given key.
**/
void IniParserHelper::setEntry(dictionary* dic, const char* key, const char* value)
{
	iniparser_set(dic,extractSectionName(key).c_str(),NULL);
	iniparser_set(dic,key,value);
}

/*******************  FUNCTION  *********************/
/**
 * Help set setup ini dic entries from boolean by converting them to string
 * internally.
 * @param dic Define the dictionnary to fill.
 * @param key Define the key to update (key:name)
 * @param value Define the boolean value to setup.
**/
void IniParserHelper::setEntry(dictionary* dic, const char* key, bool value)
{
	setEntry(dic,key,value?"true":"false");
}

/*******************  FUNCTION  *********************/
/**
 * Help set setup ini dic entries from integer by converting them to string
 * internally.
 * @param dic Define the dictionnary to fill.
 * @param key Define the key to update (key:name)
 * @param value Define the integer value to setup.
**/
void IniParserHelper::setEntry(dictionary* dic, const char* key, int value)
{
	char buffer[64];
	sprintf(buffer,"%d",value);
	setEntry(dic,key,buffer);
}

/*******************  FUNCTION  *********************/
/**
 * Help set setup ini dic entries from integer by converting them to string
 * internally.
 * @param dic Define the dictionnary to fill.
 * @param key Define the key to update (key:name)
 * @param value Define the float value to setup.
**/
void IniParserHelper::setEntry(dictionary* dic, const char* key, float value)
{
	char buffer[64];
	sprintf(buffer,"%f",value);
	setEntry(dic,key,buffer);
}


/*******************  FUNCTION  *********************/
/**
 * Need to be call once after malloc is available.
**/
Options& initGlobalOptions ( bool reinit )
{
	//error
	if (reinit == false)
		numaprofAssume (gblOptions == NULL,"initGlobalOptions was used previously, gblOptions is already init ! ");
	
	//proceed if not already done
	if (gblOptions == NULL) {
		gblOptions = new Options();
		return *gblOptions;
	} else {
		return *gblOptions;
	}
}

}
