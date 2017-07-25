/*****************************************************
			 PROJECT  : numaprof
			 VERSION  : 2.3.0
			 DATE     : 05/2017
			 AUTHOR   : Valat Sébastien - CERN
			 LICENSE  : CeCILL-C
*****************************************************/

#include "pin.H"
#include <cstdio>
#include <ProcessTracker.hpp>
#include <ThreadTracker.hpp>
#include <iostream>

using namespace std;

/********************  GLOBALS  **********************/
#if defined(TARGET_MAC222)
#define MALLOC "_malloc"
#define CALLOC "_calloc"
#define FREE "_free"
#else
#define MALLOC "malloc"
#define CALLOC "calloc"
#define FREE "free"
#endif

using namespace numaprof;

/*******************  STRUCT  ***********************/
struct ThreadData
{
	//keep track between enter/exit of malloc/calloc/... functions
	int allocSize;
	void * allocCallsite;
	//pointer to thread tracker
	ThreadTracker * tracker;
};

/*******************  GLOBALS  **********************/
ProcessTracker * gblProcessTracker = NULL;

// key for accessing TLS storage in the threads. initialized once in main()
static  TLS_KEY tls_key = INVALID_TLS_KEY;

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
static VOID RecordMemRead(VOID * ip, VOID * addr,THREADID threadid)
{
	/*if (id == 0)
	{
		id = cnt;
		cnt++;
	}
	fprintf(trace,"%p: R %p, thread %d NUMA %d thread ID %d\n", ip, addr,id,numaprof::getNumaOfPage(addr),threadid);*/
	getTls(threadid).tracker->onAccess((size_t)ip,(size_t)addr,false);
}

/*******************  FUNCTION  *********************/
// Print a memory write record
static VOID RecordMemWrite(VOID * ip, VOID * addr,THREADID threadid)
{
	/*if (id == 0)
	{
		id = cnt;
		cnt++;
	}
	fprintf(trace,"%p: W %p, thread %d NUMA %d\n", ip, addr,id,numaprof::getNumaOfPage(addr));*/
	getTls(threadid).tracker->onAccess((size_t)ip,(size_t)addr,true);
}

/*******************  FUNCTION  *********************/
static VOID beforeMalloc(ADDRINT size, VOID * retIp,THREADID threadid)
{
	ThreadData & data = getTls(threadid);
	data.allocSize = size;
	data.allocCallsite = retIp;
	//printf("malloc %lu (from %p)\n",size,retIp);
}

/*******************  FUNCTION  *********************/
static VOID afterMalloc(ADDRINT ret,THREADID threadid)
{
	//printf("    => %p\n",(void*)ret);
	ThreadData & data = getTls(threadid);
	data.tracker->onAlloc((size_t)data.allocCallsite,ret,data.allocSize);
}

/*******************  FUNCTION  *********************/
static VOID beforeCalloc(ADDRINT nmemb,ADDRINT size,VOID * retIp,THREADID threadid)
{
	ThreadData & data = getTls(threadid);
	data.allocSize = size;
	data.allocCallsite = retIp;
	//printf("calloc %lu %lu (from %p)\n",nmemb,size,retIp);
}

/*******************  FUNCTION  *********************/
static VOID beforeFree(ADDRINT ptr,THREADID threadid)
{
	//printf("free %p\n",(void*)ptr);
	ThreadData & data = getTls(threadid);
	data.tracker->onFree(ptr);
}

/*******************  FUNCTION  *********************/
static VOID beforeSchedGetAffinity(ADDRINT mask,THREADID threadid)
{
	printf("--> Intercept thread affinity (%p)!\n",(void*)mask);

	//numaprof::NumaTopo topo;
	//topo.getCurrentNumaAffinity(*(cpu_set_t*)mask);

	getTls(threadid).tracker->onSetAffinity((cpu_set_t*)mask);
}

/*******************  FUNCTION  *********************/
static void beforeFunc(void * fctAddr,THREADID threadid)
{
	//printf("Enter in %p\n",fctAddr);
}

/*******************  FUNCTION  *********************/
static void afterFunc(void * fctAddr,THREADID threadid)
{
	//printf("Exit %p\n",fctAddr);
}

/*******************  FUNCTION  *********************/
static void beforeMunmap(VOID * addr,ADDRINT size,THREADID threadid)
{
	//printf("Call munmap %p : %lu\n",addr,size);
	//printf("Enter in %p\n",fctAddr);
	getTls(threadid).tracker->onMunmap((size_t)addr,size);
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
		RTN_InsertCall(callocRtn, IPOINT_AFTER, (AFUNPTR)beforeCalloc,
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
	RTN callocRtn = RTN_FindByName(img, "munmap");
	if (RTN_Valid(callocRtn))
	{
		RTN_Open(callocRtn);
		
		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(callocRtn, IPOINT_AFTER, (AFUNPTR)beforeMunmap,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
					   IARG_THREAD_ID,IARG_END);
		RTN_Close(callocRtn);
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
static VOID instrImageSetSchedAffinity(IMG img, VOID *v)
{
	// Instrument the sched_setaffinity function to intercept thread pinning.  

	//search by name
	RTN schedRtn = RTN_FindByName(img, "sched_setaffinity");
	
	//printf(FREE " out\n");
	if (RTN_Valid(schedRtn))
	{
		RTN_Open(schedRtn);

		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(schedRtn, IPOINT_BEFORE, (AFUNPTR)beforeSchedGetAffinity,
					   IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
					   IARG_THREAD_ID,IARG_END);
		RTN_Close(schedRtn);
	}
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

	// Iterate over each memory operand of the instruction.
	for (UINT32 memOp = 0; memOp < memOperands; memOp++)
	{
		if (INS_MemoryOperandIsRead(ins, memOp) && !INS_IsStackRead(ins))
		{
			INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
				IARG_INST_PTR,
				IARG_MEMORYOP_EA, memOp,
				IARG_THREAD_ID,IARG_END);
		}
		// Note that in some architectures a single memory operand can be 
		// both read and written (for instance incl (%eax) on IA-32)
		// In that case we instrument it once for read and once for write.
		if (INS_MemoryOperandIsWritten(ins, memOp) && !INS_IsStackWrite(ins))
		{
			INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
				IARG_INST_PTR,
				IARG_MEMORYOP_EA, memOp,
				IARG_THREAD_ID,IARG_END);
		}
	}
}

/*******************  FUNCTION  *********************/
//Inspirate from code exemple of pintool doc
static VOID instrImage(IMG img, VOID *v)
{
	instrImageMmap(img,v);
	instrImageMalloc(img,v);
	instrImageCalloc(img,v);
	instrImageFree(img,v);
	instrImageSetSchedAffinity(img,v);
}

/*******************  FUNCTION  *********************/
VOID instrFunctions(RTN rtn, VOID *v)
{
	RTN_Open(rtn);

	void * addr =  (void*)RTN_Address(rtn);
	string name = RTN_Name(rtn);
	
 	if (name != ".text" && name != ".plt" && name != "__cxa_atexit" && name != "__cxa_finalize")
 	{
 		// Insert a call at the entry point of a routine to increment the call count
 		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)beforeFunc, IARG_PTR, addr, IARG_THREAD_ID,IARG_END);
 		RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)afterFunc, IARG_PTR, addr, IARG_THREAD_ID,IARG_END);
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

	//setup
	gblProcessTracker = new ProcessTracker();

	// Obtain  a key for TLS storage.
	tls_key = PIN_CreateThreadDataKey(NULL);
	if (tls_key == INVALID_TLS_KEY)
	{
		cerr << "number of already allocated keys reached the MAX_CLIENT_TLS_KEYS limit" << endl;
		PIN_ExitProcess(1);
	}

	IMG_AddInstrumentFunction(instrImage, 0);
	INS_AddInstrumentFunction(Instruction, 0);
	//RTN_AddInstrumentFunction(instrFunctions, 0);
	PIN_AddFiniFunction(Fini, 0);

	// Register ThreadStart to be called when a thread starts and stop.
	PIN_AddThreadStartFunction(ThreadStart, NULL);
	PIN_AddThreadFiniFunction(ThreadFini, NULL);

	// Never returns
	PIN_StartProgram();
	
	return 0;
}
