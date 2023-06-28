/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#include "pin.H"
#include <cstring>
#include <sys/mman.h>
#include <cstdio>
#include <core/ProcessTracker.hpp>
#include <core/ThreadTracker.hpp>
#include <portability/OS.hpp>
#include <common/Helper.hpp>
#include <common/Options.hpp>
#include <sys/syscall.h>
#include <iostream>

using namespace std;

/********************  GLOBALS  **********************/
#if defined(TARGET_MAC222)
#define MALLOC "_malloc"
#define CALLOC "_calloc"
#define FREE "_free"
#define REALLOC "_realloc"
#else
#define MALLOC "malloc"
#define CALLOC "calloc"
#define FREE "free"
#define REALLOC "realloc"
#endif

#if !defined(__NR_sched_setaffinity)
	#if defined(__x86_64__)
		#define __NR_sched_setaffinity 203
	#else
		#error "Unsupported arch !"
	#endif
#endif

//#define NUMAPROF_TRACE_ALLOCS

//HUm finally I'm not totaly sure we can do this with pintool
//See : https://github.com/wapiflapi/villoc/issues/3
#define TRACK_MALLOC true

/********************  ENUM  ************************/
enum InstrStatus
{
	INSTR_STATUS_UNKNOWN,
	INSTR_STATUS_YES,
	INSTR_STATUS_NO
};

/*******************  NAMESPACE  ********************/
using namespace numaprof;

/*******************  STRUCT  ***********************/
struct ThreadData
{
	ThreadData(void);
	//keep track between enter/exit of malloc/calloc/... functions
	int allocSize;
	void * allocCallsite;
	void * reallocPtr;
	void * newCallsite;
	//keep track between enter/exit of mmap
	size_t mmapSize;
	size_t mmapFlags;
	size_t mmapFd;
	//keep track betwen enter/exit of mremap
	void * mremapOldAddr;
	size_t mremapOldSize;
	size_t mremapNewSize;
	//keep track between enter/exit of mbind
	void * mbindAddr;
	size_t mbindLen;
	size_t mbindMode;
	void * mbindMask;
	size_t mbindNodes;
	size_t mbindFlags;
	//pointer to thread tracker
	ThreadTracker * tracker;
};

/*******************  GLOBALS  **********************/
ProcessTracker * gblProcessTracker = NULL;
bool gblHasSeenMain = true;

// key for accessing TLS storage in the threads. initialized once in main()
static  TLS_KEY tls_key = INVALID_TLS_KEY;

/*******************  FUNCTION  *********************/
ThreadData::ThreadData::ThreadData()
{
	newCallsite = NULL;
	tracker = NULL;
}

/*******************  FUNCTION  *********************/
static ThreadData & getTls(THREADID threadid)
{
	return *(static_cast<ThreadData*>(PIN_GetThreadData(tls_key, threadid)));
}

/*******************  FUNCTION  *********************/
static VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
	ThreadData * data = new ThreadData;
	if (PIN_SetThreadData(tls_key, data, threadid) == FALSE)
	{
		cerr << "PIN_SetThreadData failed" << endl;
		PIN_ExitProcess(1);
	}

	data->tracker = gblProcessTracker->createThreadTracker(threadid);
}

/*******************  FUNCTION  *********************/
static VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
	//printf("thread exit : %d\n",threadid);
	getTls(threadid).tracker->onStop();
}

/*******************  FUNCTION  *********************/
// Print a memory read record
static VOID RecordMemRead(VOID * ip, VOID * addr,bool skip,THREADID threadid)
{
	/*if (id == 0)
	{
		id = cnt;
		cnt++;
	}
	fprintf(trace,"%p: R %p, thread %d NUMA %d thread ID %d\n", ip, addr,id,numaprof::getNumaOfPage(addr),threadid);*/
	getTls(threadid).tracker->onAccess((size_t)ip,(size_t)addr,false,skip);
}

/*******************  FUNCTION  *********************/
// Print a memory write record
static VOID RecordMemWrite(VOID * ip, VOID * addr,bool skip,THREADID threadid)
{
	/*if (id == 0)
	{
		id = cnt;
		cnt++;
	}
	fprintf(trace,"%p: W %p, thread %d NUMA %d\n", ip, addr,id,numaprof::getNumaOfPage(addr));*/
	getTls(threadid).tracker->onAccess((size_t)ip,(size_t)addr,true,skip);
}

/*******************  FUNCTION  *********************/
static VOID beforeRealloc(VOID* ptr,ADDRINT size, VOID * retIp,THREADID threadid)
{
	if (gblHasSeenMain)
	{
		ThreadData & data = getTls(threadid);
		data.allocSize = size;
		data.allocCallsite = retIp;
		data.reallocPtr = ptr;
		#ifdef NUMAPROF_TRACE_ALLOCS
			printf("realloc %p %lu (from %p)\n",ptr,size,retIp);
		#endif
		data.tracker->onFree((ADDRINT)ptr);
	}
}

/*******************  FUNCTION  *********************/
static VOID afterRealloc(ADDRINT ret,THREADID threadid)
{
	if (gblHasSeenMain)
	{
		#ifdef NUMAPROF_TRACE_ALLOCS
			printf("    => %p\n",(void*)ret);
		#endif
		ThreadData & data = getTls(threadid);
		//data.tracker->onRealloc((size_t)data.allocCallsite,(size_t)data.reallocPtr,ret,data.allocSize);
		data.tracker->onAlloc((size_t)data.allocCallsite,ret,data.allocSize);
	}
}


/*******************  FUNCTION  *********************/
static VOID beforeMalloc(ADDRINT size, VOID * retIp,THREADID threadid)
{
	if (gblHasSeenMain)
	{
		ThreadData & data = getTls(threadid);
		data.allocSize = size;
		data.allocCallsite = retIp;
		#ifdef NUMAPROF_TRACE_ALLOCS
			printf("malloc %lu (from %p)\n",size,retIp);
		#endif
	}
}

/*******************  FUNCTION  *********************/
static VOID afterMalloc(ADDRINT ret,THREADID threadid)
{
	if (gblHasSeenMain)
	{
		#ifdef NUMAPROF_TRACE_ALLOCS
			printf("    => %p\n",(void*)ret);
		#endif
		ThreadData & data = getTls(threadid);
		if (data.newCallsite == NULL)
		{
			data.tracker->onAlloc((size_t)data.allocCallsite,ret,data.allocSize);
		} else {
			data.tracker->onAlloc((size_t)data.newCallsite,ret,data.allocSize);
		}
	}
}

/*******************  FUNCTION  *********************/
static VOID beforeNew(ADDRINT size, VOID * retIp,THREADID threadid)
{
	if (gblHasSeenMain)
	{
		ThreadData & data = getTls(threadid);
		if (data.newCallsite == NULL)
			data.newCallsite = retIp;
		#ifdef NUMAPROF_TRACE_ALLOCS
			printf("before new %lu (from %p)\n",size,retIp);
		#endif
	}
}

/*******************  FUNCTION  *********************/
static VOID afterNew(ADDRINT ret,THREADID threadid)
{
	if (gblHasSeenMain)
	{
		#ifdef NUMAPROF_TRACE_ALLOCS
			printf("    => %p\n",(void*)ret);
		#endif
		ThreadData & data = getTls(threadid);
		data.newCallsite = NULL;
	}
}

/*******************  FUNCTION  *********************/
static VOID beforeCalloc(ADDRINT nmemb,ADDRINT size,VOID * retIp,THREADID threadid)
{
	if (gblHasSeenMain)
	{
		ThreadData & data = getTls(threadid);
		data.allocSize = size * nmemb;
		data.allocCallsite = retIp;
		#ifdef NUMAPROF_TRACE_ALLOCS
			printf("calloc %lu %lu (from %p)\n",nmemb,size,retIp);
		#endif
	}
}

/*******************  FUNCTION  *********************/
static VOID beforeFree(ADDRINT ptr,THREADID threadid)
{
	if (gblHasSeenMain)
	{
		#ifdef NUMAPROF_TRACE_ALLOCS
			printf("free %p\n",(void*)ptr);
		#endif
		ThreadData & data = getTls(threadid);
		data.tracker->onFree(ptr);
	}
}

/*******************  FUNCTION  *********************/
static VOID beforeMBind(VOID * addr, ADDRINT len,ADDRINT mode,VOID * mask,ADDRINT nodes, ADDRINT flags,THREADID threadid)
{
	ThreadData & data = getTls(threadid);
	data.mbindAddr = addr;
	data.mbindLen = len;
	data.mbindMode = mode;
	data.mbindMask = mask;
	data.mbindNodes = nodes;
	data.mbindFlags = flags;
}

/*******************  FUNCTION  *********************/
static VOID afterMBind(THREADID threadid)
{
	ThreadData & data = getTls(threadid);
	data.tracker->onMBind(data.mbindAddr,data.mbindLen,data.mbindMode,(const unsigned long *)data.mbindMask,data.mbindNodes,data.mbindFlags);
}

/*******************  FUNCTION  *********************/
static VOID beforeSchedSetAffinity(ADDRINT pid, ADDRINT size,ADDRINT mask,THREADID threadid)
{
	if (mask == 0)
		return;
	if (gblOptions->outputSilent == false)
		fprintf(stderr,"NUMAPROF: Intercept thread affinity of %lu (%d) (%p)!\n",pid,OS::getTID(),(void*)mask);

	//numaprof::NumaTopo topo;
	//topo.getCurrentNumaAffinity(*(cpu_set_t*)mask);
	if (pid == 0 || pid == (ADDRINT)OS::getTID())
	{
		getTls(threadid).tracker->onSetAffinity((cpu_set_t*)mask,size);
	} else {
		if (getGlobalOptions().outputSilent == false)
			fprintf(stderr,"NUMAPROF: set affinity of remote thread\n");
		gblProcessTracker->onThreadSetAffinity(pid,(cpu_set_t*)mask,size);
	}
}

/*******************  FUNCTION  *********************/
static VOID SyscallEntry(THREADID threadIndex, CONTEXT *ctxt, SYSCALL_STANDARD std, VOID *v)
{
	if (PIN_GetSyscallNumber(ctxt, std) == __NR_sched_setaffinity)
		beforeSchedSetAffinity(PIN_GetSyscallArgument(ctxt, std, 0),PIN_GetSyscallArgument(ctxt, std, 1),PIN_GetSyscallArgument(ctxt, std, 2),threadIndex);
}

/*******************  FUNCTION  *********************/
static VOID beforeSetMemPolicy(ADDRINT mode, ADDRINT nodemask,ADDRINT maxnode,THREADID threadid)
{
	if (getGlobalOptions().outputSilent)
		fprintf(stderr,"NUMAPROF: Intercept thread set mem policy!\n");

	getTls(threadid).tracker->onSetMemPolicy(mode,(const unsigned long*)nodemask,maxnode);
}

/*******************  FUNCTION  *********************/
const string& Target2RtnName(ADDRINT target)
{
	const string & name = RTN_FindNameByAddress(target);

	if (name == "")
		return *new string("[Unknown routine]");
	else
		return *new string(name);
}

/*******************  FUNCTION  *********************/
const string & Target2String(ADDRINT target)
{
	static string invalid = "invalid_rtn";
    string name = RTN_FindNameByAddress(target);
    if (name == "")
        return invalid;
    else
        return *(new string(name));
}

/*******************  FUNCTION  *********************/
static BOOL IsPLT(TRACE trace)
{
    RTN rtn = TRACE_Rtn(trace);

    // All .plt thunks have a valid RTN
    if (!RTN_Valid(rtn))
        return FALSE;

    if (".plt" == SEC_Name(RTN_Sec(rtn)))
        return TRUE;
    return FALSE;
}

/*******************  FUNCTION  *********************/
static void beforeFunc(void * fctAddr,void * name,THREADID threadid)
{
	printf("Enter in %s (%p)\n",(char*)name,fctAddr);
}

/*******************  FUNCTION  *********************/
static void afterFunc(void * fctAddr,void * name,THREADID threadid)
{
	printf("Exit %s (%p)\n",(char*)name,fctAddr);
}

/*******************  FUNCTION  *********************/
static void beforeMunmap(VOID * addr,ADDRINT size,THREADID threadid)
{
	//printf("Call munmap %p : %lu\n",addr,size);
	//printf("Enter in %p\n",fctAddr);
	getTls(threadid).tracker->onMunmap((size_t)addr,size);
}

/*******************  FUNCTION  *********************/
static void beforeMmap(ADDRINT size, ADDRINT flags,ADDRINT fd,THREADID threadid)
{
	//printf("Call munmap %p : %lu\n",addr,size);
	//printf("Enter in %p\n",fctAddr);
	//getTls(threadid).tracker->onMmap((size_t)addr,size,flags,fd);
	ThreadData & data = getTls(threadid);
	data.mmapSize = size;
	data.mmapFlags = flags;
	data.mmapFd = fd;
}

/*******************  FUNCTION  *********************/
static void afterMmap(VOID * addr,THREADID threadid)
{
	//printf("Call munmap %p : %lu\n",addr,size);
	//printf("Enter in %p\n",fctAddr);
	
	ThreadData & data = getTls(threadid);
	if (addr != MAP_FAILED)
		data.tracker->onMmap((size_t)addr,data.mmapSize,data.mmapFlags,data.mmapFd);
}

/*******************  FUNCTION  *********************/
static void beforeMremap(VOID * oldAddr,ADDRINT oldSize, ADDRINT newSize,THREADID threadid)
{
	//printf("Call mrenmap %p : %lu\n",oldAddr,oldSize);
	//printf("Enter in %p\n",fctAddr);
	//getTls(threadid).tracker->onMmap((size_t)addr,size,flags,fd);
	ThreadData & data = getTls(threadid);
	data.mremapOldAddr = oldAddr;
	data.mremapOldSize = oldSize;
	data.mremapNewSize = newSize;
}

/*******************  FUNCTION  *********************/
static void afterMremap(VOID * addr,THREADID threadid)
{
	//printf("Call munmap %p : %lu\n",addr,size);
	//printf("Enter in %p\n",fctAddr);
	ThreadData & data = getTls(threadid);
	if (addr != MAP_FAILED)
		data.tracker->onMremap((size_t)data.mremapOldAddr,data.mremapOldSize,(size_t)addr,data.mremapNewSize);
}

/*******************  FUNCTION  *********************/
void A_ProcessDirectCall(ADDRINT ip, ADDRINT target, ADDRINT sp,THREADID threadid)
{
	cout << "Direct call: " << Target2String(ip) << " => " << Target2String(target) << endl;
	//callStack.ProcessCall(sp, target);
	getTls(threadid).tracker->onEnterFunction((void*)ip);
}

/*******************  FUNCTION  *********************/
void A_ProcessDirectCall_ret(ADDRINT ip, ADDRINT target, ADDRINT sp,THREADID threadid)
{
	cout << "Direct call RET: " << Target2String(ip) << " <= " << Target2String(target) << endl;
	//callStack.ProcessCall(sp, target);
	getTls(threadid).tracker->onExitFunction();
}

/*******************  FUNCTION  *********************/
void A_ProcessIndirectCall(ADDRINT ip, ADDRINT target, ADDRINT sp,THREADID threadid)
{
	cout << "Indirect call: " << Target2String(ip) << " => " << Target2String(target) << endl;
	//callStack.ProcessCall(sp, target);
	getTls(threadid).tracker->onEnterFunction((void*)ip);
}

/*******************  FUNCTION  *********************/
void A_ProcessIndirectCall_ret(ADDRINT ip, ADDRINT target, ADDRINT sp,THREADID threadid)
{
	cout << "Indirect call RET: " << Target2String(ip) << " <= " << Target2String(target) << endl;
	//callStack.ProcessCall(sp, target);
	getTls(threadid).tracker->onExitFunction();
}

/*******************  FUNCTION  *********************/
void A_ProcessStub(ADDRINT ip, ADDRINT target, ADDRINT sp,THREADID threadid) 
{
	cout << "Instrumenting stub: " << Target2String(ip) << " => " << Target2String(target) << endl;
	cout << "STUB: ";
	cout << Target2RtnName(target) << endl;
	//callStack.ProcessCall(sp, target);
	getTls(threadid).tracker->onEnterFunction((void*)ip);
}

/*******************  FUNCTION  *********************/
void A_ProcessStub_ret(ADDRINT ip, ADDRINT target, ADDRINT sp,THREADID threadid) 
{
	cout << "Instrumenting stub RET: " << Target2String(ip) << " <= " << Target2String(target) << endl;
	cout << "STUB: ";
	cout << Target2RtnName(target) << endl;
	//callStack.ProcessCall(sp, target);
	getTls(threadid).tracker->onExitFunction();
}

/*******************  FUNCTION  *********************/
static void A_ProcessReturn(ADDRINT ip, ADDRINT sp,THREADID threadid) {
	cout << "return " << Target2String(ip) <<endl;
	//callStack.ProcessReturn(sp, prevIpDoesPush);
	getTls(threadid).tracker->onExitFunction();
}

/*******************  FUNCTION  *********************/
static bool isNewOperator(const std::string & name)
{
	//std::string demangled = PIN_UndecorateSymbolName(name,UNDECORATION_COMPLETE);
	//printf("%s => %s\n",name.c_str(),demangled.c_str());
	if (strncmp(name.c_str(),"_Znw",4) == 0)
		return true;
	if (strncmp(name.c_str(),"_Zna",4) == 0)
		return true;
	return false;
}

/*******************  FUNCTION  *********************/
VOID beforeMain(void)
{
	gblHasSeenMain = true;
}

/*******************  FUNCTION  *********************/
VOID instrImageLoad(IMG img,VOID * v)
{
	gblProcessTracker->registerLibBaseAddr(IMG_Name(img), IMG_LowAddress(img));
	gblProcessTracker->markObjectFiledAsNotPinned();
}

/*******************  FUNCTION  *********************/
VOID instrImageNew(IMG img, VOID *v)
{
	// Instrument the malloc() and free() functions.  Print the input argument
	// of each malloc() or free(), and the return value of malloc().
	//
	//  Find the new() function.
	for( SEC sec= IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec) )
	{
		for( RTN rtn= SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn) )
		{
			std::string name = RTN_Name(rtn);
			if (isNewOperator(name))
			{
				//printf("Instument NEW %s\n",name.c_str());
				RTN_Open(rtn);
				
				// Instrument malloc() to print the input argument value and the return value.
				RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)beforeNew,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
							IARG_RETURN_IP, IARG_THREAD_ID,IARG_END);
				
				// Instrument malloc() to print the input argument value and the return value.
				RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)afterNew,
							IARG_FUNCRET_EXITPOINT_VALUE,
							IARG_THREAD_ID,IARG_END);
				RTN_Close(rtn);
			}
		}
	}
}

/*******************  FUNCTION  *********************/
static VOID instrImageMalloc(IMG img, VOID *v)
{
	// Instrument the malloc() and free() functions.  Print the input argument
	// of each malloc() or free(), and the return value of malloc().
	//
	//  Find the malloc() function.
	RTN mallocRtn = RTN_FindByName(img, MALLOC);
	if (RTN_Valid(mallocRtn))
	{
		if (!getGlobalOptions().outputSilent)
			fprintf(stderr,"NUMAPROF: instr malloc from %s\n",IMG_Name(img).c_str());
		RTN_Open(mallocRtn);
		
		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(mallocRtn, IPOINT_BEFORE, (AFUNPTR)beforeMalloc,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
					   IARG_RETURN_IP, IARG_THREAD_ID,IARG_END);
		
		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(mallocRtn, IPOINT_AFTER, (AFUNPTR)afterMalloc,
					   IARG_FUNCRET_EXITPOINT_VALUE,
					   IARG_THREAD_ID,IARG_END);
		RTN_Close(mallocRtn);
	}
}

/*******************  FUNCTION  *********************/
static VOID instrImageRealloc(IMG img, VOID *v)
{
	// Instrument the malloc() and free() functions.  Print the input argument
	// of each malloc() or free(), and the return value of malloc().
	//
	//  Find the malloc() function.
	RTN reallocRtn = RTN_FindByName(img, REALLOC);
	if (RTN_Valid(reallocRtn))
	{
		RTN_Open(reallocRtn);
		
		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(reallocRtn, IPOINT_BEFORE, (AFUNPTR)beforeRealloc,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
					   IARG_RETURN_IP, IARG_THREAD_ID,IARG_END);
		
		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(reallocRtn, IPOINT_AFTER, (AFUNPTR)afterRealloc,
					   IARG_FUNCRET_EXITPOINT_VALUE,
					   IARG_THREAD_ID,IARG_END);
		RTN_Close(reallocRtn);
	}
}

/*******************  FUNCTION  *********************/
static VOID instrImageCalloc(IMG img, VOID *v)
{
	// Instrument the malloc() and free() functions.  Print the input argument
	// of each malloc() or free(), and the return value of malloc().
	//
	//  Find the malloc() function.
	RTN callocRtn = RTN_FindByName(img, CALLOC);
	if (RTN_Valid(callocRtn))
	{
		RTN_Open(callocRtn);
		
		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(callocRtn, IPOINT_BEFORE, (AFUNPTR)beforeCalloc,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
					   IARG_RETURN_IP, IARG_THREAD_ID,IARG_END);

		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(callocRtn, IPOINT_AFTER, (AFUNPTR)afterMalloc,
					   IARG_FUNCRET_EXITPOINT_VALUE,
					   IARG_THREAD_ID,IARG_END);
		RTN_Close(callocRtn);
	}
}

/*******************  FUNCTION  *********************/
static VOID instrImageMmap(IMG img, VOID *v)
{
	// Instrument the malloc() and free() functions.  Print the input argument
	// of each malloc() or free(), and the return value of malloc().
	//
	//  Find the malloc() function.
	RTN mmapRtn = RTN_FindByName(img, "mmap");
	if (RTN_Valid(mmapRtn))
	{
		RTN_Open(mmapRtn);
		
		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(mmapRtn, IPOINT_BEFORE, (AFUNPTR)beforeMmap,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 4,
					   IARG_THREAD_ID,IARG_END);
		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(mmapRtn, IPOINT_AFTER, (AFUNPTR)afterMmap,
					   IARG_FUNCRET_EXITPOINT_VALUE,
					   IARG_THREAD_ID,IARG_END);
		RTN_Close(mmapRtn);
	}
	
	RTN munapRtn = RTN_FindByName(img, "munmap");
	if (RTN_Valid(munapRtn))
	{
		RTN_Open(munapRtn);
		
		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(munapRtn, IPOINT_AFTER, (AFUNPTR)beforeMunmap,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
					   IARG_THREAD_ID,IARG_END);
		RTN_Close(munapRtn);
	}
	
	RTN mremapRtn = RTN_FindByName(img, "mremap");
	if (RTN_Valid(mremapRtn))
	{
		RTN_Open(mremapRtn);
		
		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(mremapRtn, IPOINT_BEFORE, (AFUNPTR)beforeMremap,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
					   IARG_THREAD_ID,IARG_END);
		RTN_InsertCall(mremapRtn, IPOINT_AFTER, (AFUNPTR)afterMremap,
					   IARG_FUNCRET_EXITPOINT_VALUE,
					   IARG_THREAD_ID,IARG_END);
		RTN_Close(mremapRtn);
	}
}

/*******************  FUNCTION  *********************/
static VOID instrImageFree(IMG img, VOID *v)
{
	// Instrument the malloc() and free() functions.  Print the input argument
	// of each malloc() or free(), and the return value of malloc().
	//
	//  Find the malloc() function.
	RTN freeRtn = RTN_FindByName(img, FREE);
	
	//printf(FREE " out\n");
	if (RTN_Valid(freeRtn))
	{
		RTN_Open(freeRtn);

		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(freeRtn, IPOINT_BEFORE, (AFUNPTR)beforeFree,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
					   IARG_THREAD_ID,IARG_END);
		RTN_Close(freeRtn);
	}
}

/*******************  FUNCTION  *********************/
static VOID instrImageMain(IMG img, VOID *v)
{
	// Instrument the malloc() and free() functions.  Print the input argument
	// of each malloc() or free(), and the return value of malloc().
	//
	//  Find the malloc() function.
	RTN mainRtn = RTN_FindByName(img, "_init");
	
	//printf(FREE " out\n");
	if (RTN_Valid(mainRtn))
	{
		RTN_Open(mainRtn);

		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(mainRtn, IPOINT_BEFORE, (AFUNPTR)beforeMain,IARG_END);
		RTN_Close(mainRtn);
	}
}

/*******************  FUNCTION  *********************/
static VOID instrImageSetMempolicy(IMG img, VOID *v)
{
	// Instrument the sched_setaffinity function to intercept thread pinning.  

	//search by name
	RTN setMemPolicyRtn = RTN_FindByName(img, "set_mempolicy");
	
	//printf(FREE " out\n");
	if (RTN_Valid(setMemPolicyRtn))
	{
		RTN_Open(setMemPolicyRtn);

		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(setMemPolicyRtn, IPOINT_BEFORE, (AFUNPTR)beforeSetMemPolicy,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
					   IARG_THREAD_ID,IARG_END);
		RTN_Close(setMemPolicyRtn);
	}
}

/*******************  FUNCTION  *********************/
static VOID instrImageMBind(IMG img, VOID *v)
{
	// Instrument the sched_setaffinity function to intercept thread pinning.  

	//search by name
	RTN mbindRtn = RTN_FindByName(img, "mbind");
	
	//printf(FREE " out\n");
	if (RTN_Valid(mbindRtn))
	{
		RTN_Open(mbindRtn);

		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(mbindRtn, IPOINT_BEFORE, (AFUNPTR)beforeMBind,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 4,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 5,
					   IARG_THREAD_ID,IARG_END);
		
		RTN_InsertCall(mbindRtn, IPOINT_AFTER, (AFUNPTR)afterMBind,
					   IARG_THREAD_ID,IARG_END);
		RTN_Close(mbindRtn);
	}
}

/*******************  FUNCTION  *********************/
//Not needed anymore as we use direct instrumentation of syscall
//required to support intel opemp.
//But could be better if we can move back to function instrumentation
//instead of instrumenting all syscalls....
/*static VOID instrImageSetSchedAffinity(IMG img, VOID *v)
{
	// Instrument the sched_setaffinity function to intercept thread pinning.  

	//search by name
	RTN schedRtn = RTN_FindByName(img, "sched_setaffinity");
	
	//printf(FREE " out\n");
	if (RTN_Valid(schedRtn))
	{
		RTN_Open(schedRtn);

		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(schedRtn, IPOINT_BEFORE, (AFUNPTR)beforeSchedSetAffinity,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
					   IARG_THREAD_ID,IARG_END);
		RTN_Close(schedRtn);
	}
}*/

/*******************  FUNCTION  *********************/
static InstrStatus checkInstrumentInstruction(INS ins)
{
	//extract binary
	ADDRINT addr = INS_Address (ins);
	IMG img = IMG_FindByAddress(addr);
	
	//ref
	const std::vector<std::string> & vec = getGlobalOptions().coreSkipBinariesVect;
	
	if (IMG_Valid(img))
	{
		for (std::vector<std::string>::const_iterator it = vec.begin() ; it != vec.end() ; ++it)
			if (Helper::contain(IMG_Name(img).c_str(),it->c_str()))
				return INSTR_STATUS_NO;
	}
	
	return INSTR_STATUS_YES;
}

/*******************  FUNCTION  *********************/
// Is called for every instruction and instruments reads and writes
static VOID Instruction(INS ins, VOID *v)
{
	// Instruments memory accesses using a predicated call, i.e.
	// the instrumentation is called iff the instruction will actually be executed.
	//
	// On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
	// prefixed instructions appear as predicated instructions in Pin.
	UINT32 memOperands = INS_MemoryOperandCount(ins);
	
	//check default status
	InstrStatus status = INSTR_STATUS_UNKNOWN;
	if (getGlobalOptions().coreSkipBinariesVect.empty())
		status = INSTR_STATUS_YES;

	// Iterate over each memory operand of the instruction.
	for (UINT32 memOp = 0; memOp < memOperands; memOp++)
	{
		if (INS_MemoryOperandIsRead(ins, memOp) && (!INS_IsStackRead(ins) || !gblOptions->coreSkipStackAccesses))
		{
			//check
			if (status == INSTR_STATUS_UNKNOWN)
				status = checkInstrumentInstruction(ins);
			
			INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
				IARG_INST_PTR,
				IARG_MEMORYOP_EA, memOp,
				IARG_BOOL,(bool)(status == INSTR_STATUS_NO),
				IARG_THREAD_ID,IARG_END);
		}
		// Note that in some architectures a single memory operand can be 
		// both read and written (for instance incl (%eax) on IA-32)
		// In that case we instrument it once for read and once for write.
		if (INS_MemoryOperandIsWritten(ins, memOp) && (!INS_IsStackWrite(ins) || !gblOptions->coreSkipStackAccesses))
		{
			//check
			if (status == INSTR_STATUS_UNKNOWN)
				status = checkInstrumentInstruction(ins);
			
			INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
				IARG_INST_PTR,
				IARG_MEMORYOP_EA, memOp,
				IARG_BOOL,(bool)(status == INSTR_STATUS_NO),
				IARG_THREAD_ID,IARG_END);
		}
	}

	/*if( INS_IsCall(ins) ) {
		if( INS_IsDirectBranchOrCall(ins) ) {
			ADDRINT target = INS_DirectBranchOrCallTargetAddress(ins);
			INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
										(AFUNPTR)A_ProcessDirectCall,
										IARG_INST_PTR,
										IARG_ADDRINT, target,
										IARG_REG_VALUE, REG_STACK_PTR,
										IARG_END);
		} else if( !INS_IsPlt(ins) ) {
			INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
										(AFUNPTR)A_ProcessIndirectCall,
										IARG_INST_PTR,
										IARG_BRANCH_TARGET_ADDR,
										IARG_REG_VALUE, REG_STACK_PTR,
										IARG_END);
		}
	}*/
	/*if( INS_IsPlt(ins) ) {
		INS_InsertCall(ins, IPOINT_BEFORE, 
						(AFUNPTR)A_ProcessStub,
						IARG_INST_PTR,
						IARG_BRANCH_TARGET_ADDR,
						IARG_REG_VALUE, REG_STACK_PTR,
						IARG_END);
	}*/
	/*if( INS_IsRet(ins) ) {
		INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
									(AFUNPTR)A_ProcessReturn,
									IARG_INST_PTR,
									IARG_REG_VALUE, REG_STACK_PTR,
									IARG_END);

	}*/
}

/*******************  FUNCTION  *********************/
void I_Trace(TRACE trace, void *v)
{
    //FIXME if (PIN_IsSignalHandler()) {Sequence_ProcessSignalHandler(head)};

    for(BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {

        INS tail = BBL_InsTail(bbl);
        
        // All calls and returns
        if( INS_IsSyscall(tail) ) {
            /*INS_InsertPredicatedCall(tail, IPOINT_BEFORE,
                                     (AFUNPTR)A_ProcessSyscall,
                                     IARG_INST_PTR,
                                     IARG_SYSCALL_NUMBER,
                                     IARG_REG_VALUE, REG_STACK_PTR,
                                     IARG_SYSCALL_ARG0,
                                     IARG_END);*/
        } else {
            if( INS_IsCall(tail) ) {
                if( INS_IsDirectControlFlow(tail) ) {
                    ADDRINT target = INS_DirectControlFlowTargetAddress(tail);
                    INS_InsertPredicatedCall(tail, IPOINT_BEFORE,
                                             (AFUNPTR)A_ProcessDirectCall,
                                             IARG_INST_PTR,
                                             IARG_ADDRINT, target,
                                             IARG_REG_VALUE, REG_STACK_PTR,
                                             IARG_THREAD_ID,
                                             IARG_END);
					/*INS_InsertPredicatedCall(tail, IPOINT_AFTER,
                                             (AFUNPTR)A_ProcessDirectCall_ret,
                                             IARG_INST_PTR,
                                             IARG_ADDRINT, target,
                                             IARG_REG_VALUE, REG_STACK_PTR,
											 IARG_CALL_ORDER, CALL_ORDER_LAST,
                                             IARG_END);*/
                } else if( !IsPLT(trace) ) {
                    INS_InsertPredicatedCall(tail, IPOINT_BEFORE,
                                             (AFUNPTR)A_ProcessIndirectCall,
                                             IARG_INST_PTR,
                                             IARG_BRANCH_TARGET_ADDR,
                                             IARG_REG_VALUE, REG_STACK_PTR,
                                             IARG_THREAD_ID,
											 IARG_END);
					/*INS_InsertPredicatedCall(tail, IPOINT_AFTER,
                                             (AFUNPTR)A_ProcessIndirectCall_ret,
											 IARG_CALL_ORDER, CALL_ORDER_LAST,
                                             IARG_INST_PTR,
                                             IARG_BRANCH_TARGET_ADDR,
                                             IARG_REG_VALUE, REG_STACK_PTR,
                                             IARG_END);*/
                }
            }
            if( IsPLT(trace) ) {
                INS_InsertCall(tail, IPOINT_BEFORE, 
                               (AFUNPTR)A_ProcessStub,
                               IARG_INST_PTR,
                               IARG_BRANCH_TARGET_ADDR,
                               IARG_REG_VALUE, REG_STACK_PTR,
                               IARG_THREAD_ID,
							   IARG_END);
				/*INS_InsertCall(tail, IPOINT_AFTER, 
                               (AFUNPTR)A_ProcessStub_ret,
							   IARG_CALL_ORDER, CALL_ORDER_LAST,
                               IARG_INST_PTR,
                               IARG_BRANCH_TARGET_ADDR,
                               IARG_REG_VALUE, REG_STACK_PTR,
                               IARG_END);*/
            }
            if( INS_IsRet(tail) ) {
                INS_InsertPredicatedCall(tail, IPOINT_BEFORE,
                                         (AFUNPTR)A_ProcessReturn,
                                         IARG_INST_PTR,
                                         IARG_REG_VALUE, REG_STACK_PTR,
                                         IARG_THREAD_ID,
										 IARG_END);
	
            }
        }
    }
}

/*******************  FUNCTION  *********************/
//Inspirate from code exemple of pintool doc
static VOID instrImage(IMG img, VOID *v)
{
	instrImageMmap(img,v);
	if (TRACK_MALLOC)
	{
		//we do not instrument ld-linux mallocs
		if (Helper::contain(IMG_Name(img).c_str(),"ld-linux") == false)
		{
			instrImageMalloc(img,v);
			instrImageNew(img,v);
			instrImageRealloc(img,v);
			instrImageCalloc(img,v);
			instrImageFree(img,v);
		}
		if (false) 
			instrImageMain(img,v);
	}
	//we use capture of syscall directly (required to intel OpenMP)
	//instrImageSetSchedAffinity(img,v);
	instrImageMBind(img,v);
	instrImageSetMempolicy(img,v);
}

/*******************  FUNCTION  *********************/
VOID instrFunctions(RTN rtn, VOID *v)
{
	RTN_Open(rtn);

	void * addr =  (void*)RTN_Address(rtn);
	string name = RTN_Name(rtn);
	char * strName = strdup(name.c_str());
	
 	if (name != ".text" && name != ".plt" && name != "__cxa_atexit" && name != "__cxa_finalize")
 	{
 		// Insert a call at the entry point of a routine to increment the call count
 		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)beforeFunc, IARG_PTR, addr,IARG_PTR, strName, IARG_THREAD_ID,IARG_END);
 		RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)afterFunc, IARG_PTR, addr,IARG_PTR, strName, IARG_THREAD_ID,IARG_END);
		//gblState.names.setupNewEntry(addr,RTN_Name(rtn));
 	}

	RTN_Close(rtn);
}


/*******************  FUNCTION  *********************/
static VOID Fini(INT32 code, VOID *v)
{
	gblProcessTracker->onExit();
}

/*******************  FUNCTION  *********************/
//Print Help Message
static INT32 Usage()
{
	PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
			  + KNOB_BASE::StringKnobSummary() + "\n");
	return -1;
}

/*******************  FUNCTION  *********************/
int main(int argc, char *argv[])
{
	//setup symbols (required to instr malloc/free)
	PIN_InitSymbols();

	if (PIN_Init(argc, argv)) return Usage();
	
	//load options
	Options & options = initGlobalOptions();
	const char * configFile = getenv("NUMAPROF_CONFIG");
	if (configFile != NULL)
		options.loadFromFile(configFile);
	const char * envOptions = getenv("NUMAPROF_OPTIONS");
	if (envOptions != NULL)
		options.loadFromString(envOptions);
	options.cache();

	//debug (can be usefull if config detection fails)
	if (options.outputSilent == false)
		fprintf(stderr,"NUMAPROF: Building process tracker...\n");

	//setup
	gblProcessTracker = new ProcessTracker();

	//debug (can be usefull if config detection fails)
	if (options.outputSilent == false)
		fprintf(stderr,"NUMAPROF: Building TLS...\n");

	// Obtain  a key for TLS storage.
	tls_key = PIN_CreateThreadDataKey(NULL);
	if (tls_key == INVALID_TLS_KEY)
	{
		cerr << "number of already allocated keys reached the MAX_CLIENT_TLS_KEYS limit" << endl;
		PIN_ExitProcess(1);
	}

	//debug (can be usefull if config detection fails)
	if (options.outputSilent == false)
		fprintf(stderr,"NUMAPROF: Instrumenting the binary...\n");

	IMG_AddInstrumentFunction(instrImageLoad,0);
	IMG_AddInstrumentFunction(instrImage, 0);
	INS_AddInstrumentFunction(Instruction, 0);
	#ifdef NUMAPROG_CALLSTACK
		TRACE_AddInstrumentFunction(I_Trace, 0);
	#endif
	//RTN_AddInstrumentFunction(instrFunctions, 0);
	PIN_AddFiniFunction(Fini, 0);

	// Register ThreadStart to be called when a thread starts and stop.
	PIN_AddThreadStartFunction(ThreadStart, NULL);
	PIN_AddThreadFiniFunction(ThreadFini, NULL);
	PIN_AddSyscallEntryFunction(SyscallEntry, NULL);

	//debug (can be usefull if config detection fails)
	if (options.outputSilent == false)
		fprintf(stderr,"NUMAPROF: Start program...\n");

	// Never returns
	PIN_StartProgram();
	
	return 0;
}
