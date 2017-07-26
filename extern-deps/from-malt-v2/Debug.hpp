/*****************************************************
             PROJECT  : MATT
             VERSION  : 0.1.0-dev
             DATE     : 02/2015
             AUTHOR   : Valat Sébastien
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef MATT_DEBUG_HPP
#define MATT_DEBUG_HPP

/********************  HEADERS  *********************/
#include "FormattedMessage.hpp"

/*******************  NAMESPACE  ********************/
namespace MATT
{

/*********************  ENUM  ***********************/
/**
 * @brief Define the debug message levels.
**/
enum DebugLevel
{
	/** For assertions, but your might use standard assert() instead. **/
	MESSAGE_ASSERT,
	/** Used to print some debug informations. **/
	MESSAGE_DEBUG,
	/** Used to print some informations. **/
	MESSAGE_INFO,
	/** Used for normal printing of formatted messages. **/
	MESSAGE_NORMAL,
	/** Used to print warnings. **/
	MESSAGE_WARNING,
	/** Used to print errors. **/
	MESSAGE_ERROR,
	/** Used to print fatal error and abort(). **/
	MESSAGE_FATAL
};

/*********************  CLASS  **********************/
/**
 * @brief Base class to manage errors.
 * 
 * The debug class provide a short way to format error, debug or warning messages and
 * manage error final state (abort, exception...).
 * 
@code{.cpp}
Debug("My message as ard %1 and %2 !",MESSAGE_FATAL).arg(value1).arg(value2).end();
@endcode
**/
class Debug : public FormattedMessage
{
	public:
		Debug(const char * format,const char * file,int line,DebugLevel level = MESSAGE_DEBUG);
		Debug(const char * format,DebugLevel level = MESSAGE_DEBUG);
		virtual ~Debug(void);
		virtual void end(void);
	protected:
		/** Store the message level for end() method. **/
		DebugLevel level;
		/** Store file location to inform user in end() method. **/
		std::string file;
		/** Store line to inform user in end() method. **/
		int line;
		/** To check is message is emitted at exit (only for debug purpose). **/
		bool emitted;
};

/*********************  CLASS  **********************/
/**
 * Provide the same interface than Debug class but do nothing in implementation.
 * It is used to ignore debug message at compile time if NDEBUG macro is enabled.
**/
class DebugDummy
{
	public:
		DebugDummy(const char * format,const char * file,int line,DebugLevel level = MESSAGE_DEBUG, const char * category = NULL){};
		DebugDummy(const char * format,DebugLevel level = MESSAGE_DEBUG,const char * category = NULL){};
		static void enableCat(const std::string & cat){};
		static void enableAll(void){};
		static bool showCat(const char * cat){return false;};
		static void showList(void){};
		static void setVerbosity(const std::string & value){};
		//from message class
		template <class T> DebugDummy & arg(const T & value) {return *this;};
		DebugDummy & arg(const std::string & value) {return *this;};
		DebugDummy & arg(const char * value) {return *this;};
		DebugDummy & argStrErrno(void) {return *this;};
		DebugDummy & argUnit(unsigned long value,const char * unit = "") {return *this;};
		std::string toString(void) const {return "";};
		void toStream(std::ostream & out) const {};
		void end(void){};
};

/********************  MACROS  **********************/
/**
 * Helper to get current location (file and line).
**/
#define MATT_CODE_LOCATION __FILE__,__LINE__

/*******************  FUNCTION  *********************/
/** Short function to print debug static messages without arguments. **/
inline Debug debug(const char * format)   {return Debug(format,MESSAGE_DEBUG);  }
/** Short function to print warning static messages without arguments. **/
inline Debug warning(const char * format) {return Debug(format,MESSAGE_WARNING);}
/** Short function to print error static messages without arguments. **/
inline Debug error(const char * format)   {return Debug(format,MESSAGE_ERROR);  }
/** Short function to print fatal static messages without arguments. **/
inline Debug fatal(const char * format)   {return Debug(format,MESSAGE_FATAL);  }

/********************  MACROS  **********************/
#define MATT_FATAL_ARG(x)   MATT::Debug(x,MATT_CODE_LOCATION,MATT::MESSAGE_FATAL  )
#define MATT_ERROR_ARG(x)   MATT::Debug(x,MATT_CODE_LOCATION,MATT::MESSAGE_ERROR  )
#define MATT_WARNING_ARG(x) MATT::Debug(x,MATT_CODE_LOCATION,MATT::MESSAGE_WARNING)
#define MATT_MESSAGE_ARG(x) MATT::Debug(x,MATT_CODE_LOCATION,MATT::MESSAGE_NORMAL )
#define MATT_INFO_ARG(x)    MATT::Debug(x,MATT_CODE_LOCATION,MATT::MESSAGE_INFO )
#ifdef NDEBUG
	#define MALT_DEBUG_ARG(cat,x) MALT::DebugDummy(x,MALT_CODE_LOCATION,MALT::MESSAGE_DEBUG,cat)
#else
	#define MALT_DEBUG_ARG(cat,x) MALT::Debug(x,MALT_CODE_LOCATION,MALT::MESSAGE_DEBUG,cat)
#endif

/********************  MACROS  **********************/
#define MATT_FATAL(x)   MATT_FATAL_ARG(x).end()
#define MATT_DEBUG(x)   MATT_DEBUG_ARG(x).end()
#define MATT_ERROR(x)   MATT_ERROR_ARG(x).end()
#define MATT_WARNING(x) MATT_WARNING_ARG(x).end()
#define MATT_MESSAGE(x) MATT_MESSAGE_ARG(x).end()
#define MATT_INFO(x)    MATT_INFO_ARG(x).end()

/********************  MACROS  **********************/
#define assume(check,message) do { if (!(check)) MATT_FATAL(message); } while(0)
#define assumeArg(check,message) if (!(check)) MATT_FATAL_ARG(message)

/********************  MACROS  **********************/
#define MATT_TO_STRING(x) #x
#ifdef NDEBUG
	#define MATT_ASSERT(x)      do{} while(0)
#else
	#define MATT_ASSERT(x)      do{ if (!(x)) MATT::Debug(MATT_TO_STRING(x),MATT_CODE_LOCATION,MATT::MESSAGE_ASSERT).end(); } while(0)
#endif

}

#endif //MATT_DEBUG_HPP
