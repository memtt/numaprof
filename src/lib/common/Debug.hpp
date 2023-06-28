/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

//Imported from mpc_numaprofator_cpp : https://github.com/svalat/mpc_allocator_cpp
//from same author.

#ifndef DEBUG_HPP
#define DEBUG_HPP

/********************  NAMESPACE  *******************/
namespace numaprof
{


/********************  MACRO  ***********************/
//Define colors
#ifdef HAVE_COLOR
	#define DEFAULT_COLOR    "\033[0m"
	#define COLOR_RED        "\033[31m"
	#define COLOR_GREEN      "\033[32m"
	#define COLOR_YELLOW     "\033[33m"
	#define COLOR_BLUE       "\033[34m"
	#define COLOR_MAGENTA    "\033[35m"
	#define COLOR_CYAN       "\033[36m"
	#define COLOR_WHITE      "\033[37m"
#else
	#define DEFAULT_COLOR    ""
	#define COLOR_RED        ""
	#define COLOR_GREEN      ""
	#define COLOR_YELLOW     ""
	#define COLOR_BLUE       ""
	#define COLOR_MAGENTA    ""
	#define COLOR_CYAN       ""
	#define COLOR_WHITE      ""
#endif

/********************  ENUM  ************************/
/** Define message levels **/
enum MessageLevel
{
	MESSAGE_DEBUG        = 0,
	MESSAGE_INFO         = 1,
	MESSAGE_TRACE        = 2,
	MESSAGE_WARNING      = 3,
	MESSAGE_ERROR        = 4,
	MESSAGE_PERROR       = 5,
	MESSAGE_FATAL        = 6,
	MESSAGE_FATAL_PERROR = 7,
	MESSAGE_ASSERT       = 8
};

/*********************  STRUCT  *********************/
/**
 * Debug message class to format error and debug messages.
**/
class DebugMessage
{
	public:
		DebugMessage(MessageLevel level,const char * file, const char * func,int line);
		void print(const char * format,...);
	private:
		/** Source file **/
		const char * file;
		/** Function in source file **/
		const char * func;
		/** Lien in source file **/
		int line;
		/** Message lelvel **/
		MessageLevel level;
};

/********************  MACRO  ***********************/
/** Macro to extract code location **/
#define MESSAGE_CODE_LOCATION __FILE__, __FUNCTION__, __LINE__

/********************  MACRO  ***********************/
#ifndef NDEBUG
	/** 
	 * Debug macro to check values, prefer this macro to assert as it handle in malloc stuff 
	 * @param x Value to check.
	**/
	#define numaprofAssert(x)      do { if (!(x)) numaprof::DebugMessage(numaprof::MESSAGE_ASSERT , MESSAGE_CODE_LOCATION).print("%s",#x);  } while(0)
	/** 
	 * Conditional debug message.
	 * @param x Value to check.
	 * @param m Message to print if value is true.
	**/
	#define numaprofCondDebug(x,m) do { if (!(x)) numaprof::DebugMessage(numaprof::MESSAGE_DEBUG  , MESSAGE_CODE_LOCATION).print("%s",(m)); } while(0)
	/**
	 * Print debug message.
	 * @param m Message to print.
	**/
	#define numaprofDebug          numaprof::DebugMessage(numaprof::MESSAGE_DEBUG, MESSAGE_CODE_LOCATION).print
#else
	/** Fake version when build in release mode. **/
	#define numaprofAssert( x )    do{} while(0)
	/** Fake version when build in release mode. **/
	#define numaprofCondDebug(x,m) do{} while(0)
	/** Fake version when build in release mode. **/
	#define numaprofDebug(...)     do{} while(0)
#endif

/********************  MACRO  ***********************/
#define TRACE
#ifdef TRACE
	/** Trace mode **/
	#define numaprofTrace          numaprof::DebugMessage(numaprof::MESSAGE_TRACE, MESSAGE_CODE_LOCATION).print
#else
	/** Fake version when build in none trace mode. **/
	#define numaprofTrace(...)     do{} while(0)
#endif

/********************  MACRO  ***********************/
/** To be called on gatal error, provide format and parameters like printf **/
#define numaprofFatal       numaprof::DebugMessage(numaprof::MESSAGE_FATAL       , MESSAGE_CODE_LOCATION).print
/** To be called on error, provide format and parameters like printf **/
#define numaprofFatalPerror numaprof::DebugMessage(numaprof::MESSAGE_FATAL_PERROR, MESSAGE_CODE_LOCATION).print
/** To be called on libc error, provide format and parameters like printf **/
#define numaprofPerror      numaprof::DebugMessage(numaprof::MESSAGE_PERROR      , MESSAGE_CODE_LOCATION).print
/** To be called on warning, provide format and parameters like printf **/
#define numaprofWarning     numaprof::DebugMessage(numaprof::MESSAGE_WARNING     , MESSAGE_CODE_LOCATION).print

/********************  MACRO  ***********************/
/** Used to check values and print a message if not true. Similar to numaprofAssert but will be kept even in release. **/
#define numaprofAssume(x,m)        do { if (!(x)) numaprofFatal((m));       } while(0)
/** Used to check values and print a message if not true. Similar to numaprofAssert but will be kept even in release. **/
#define numaprofAssumePerror(x,m)  do { if (!(x)) numaprofFatalPerror((m)); } while(0)
/** Used to check values and print a message if not true. Similar to numaprofAssert but will be kept even in release. **/
#define numaprofCondWarning(x,m)   do { if (!(x)) numaprofWarning((m));     } while(0)

/********************  MACRO  ***********************/
/** To avoid some compiler warnings **/
#define numaprofUnused(x)          ((void)x)

}

#endif //DEBUG_HPP
