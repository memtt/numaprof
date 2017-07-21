/*****************************************************
             PROJECT  : numlaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

//Imported from mpc_numaprofator_cpp : https://github.com/svalat/mpc_numaprofator_cpp
//from same author.

#ifndef DEBUG_HPP
#define DEBUG_HPP

/********************  NAMESPACE  *******************/
namespace numaprof
{


/********************  MACRO  ***********************/
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
class DebugMessage
{
	public:
		DebugMessage(MessageLevel level,const char * file, const char * func,int line);
		void print(const char * format,...);
	private:
		const char * file;
		const char * func;
		int line;
		MessageLevel level;
};

/********************  MACRO  ***********************/
#define MESSAGE_CODE_LOCATION __FILE__, __FUNCTION__, __LINE__

/********************  MACRO  ***********************/
#ifndef NDEBUG
	#define numaprofAssert(x)      do { if (!(x)) numaprof::DebugMessage(numaprof::MESSAGE_ASSERT , MESSAGE_CODE_LOCATION).print("%s",#x);  } while(0)
	#define numaprofCondDebug(x,m) do { if (!(x)) numaprof::DebugMessage(numaprof::MESSAGE_DEBUG  , MESSAGE_CODE_LOCATION).print("%s",(m)); } while(0)
	#define numaprofDebug          numaprof::DebugMessage(numaprof::MESSAGE_DEBUG, MESSAGE_CODE_LOCATION).print
#else
	#define numaprofAssert( x )    do{} while(0)
	#define numaprofCondDebug(x,m) do{} while(0)
	#define numaprofDebug(...)     do{} while(0)
#endif

/********************  MACRO  ***********************/
#define TRACE
#ifdef TRACE
	#define numaprofTrace          numaprof::DebugMessage(numaprof::MESSAGE_TRACE, MESSAGE_CODE_LOCATION).print
#else
	#define numaprofTrace(...)     do{} while(0)
#endif

/********************  MACRO  ***********************/
#define numaprofFatal       numaprof::DebugMessage(numaprof::MESSAGE_FATAL       , MESSAGE_CODE_LOCATION).print
#define numaprofFatalPerror numaprof::DebugMessage(numaprof::MESSAGE_FATAL_PERROR, MESSAGE_CODE_LOCATION).print
#define numaprofPerror      numaprof::DebugMessage(numaprof::MESSAGE_PERROR      , MESSAGE_CODE_LOCATION).print
#define numaprofWarning     numaprof::DebugMessage(numaprof::MESSAGE_WARNING     , MESSAGE_CODE_LOCATION).print

/********************  MACRO  ***********************/
#define numaprofAssume(x,m)        do { if (!(x)) numaprofFatal((m));       } while(0)
#define numaprofAssumePerror(x,m)  do { if (!(x)) numaprofFatalPerror((m)); } while(0)
#define numaprofCondWarning(x,m)   do { if (!(x)) numaprofWarning((m));     } while(0)

/********************  MACRO  ***********************/
#define numaprofUnused(x)          ((void)x)

};

#endif //DEBUG_HPP
