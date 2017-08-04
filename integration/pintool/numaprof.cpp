/*****************************************************
			 PROJECT  : numaprof
			 VERSION  : 2.3.0
			 DATE     : 05/2017
			 AUTHOR   : Valat Sébastien - CERN
			 LICENSE  : CeCILL-C
*****************************************************/

#include "pin.H"
#include <sys/mman.h>
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
#define REALLOC "_realloc"
#else
#define MALLOC "malloc"
#define CALLOC "calloc"
#define FREE "free"
#define REALLOC "realloc"
#endif

using namespace numaprof;

/*******************  STRUCT  ***********************/
struct ThreadData
{
	//keep track between enter/exit of malloc/calloc/... functions
	int allocSize;
	void * allocCallsite;
	void * reallocPtr;
	//keep track between enter/exit of mmap
	size_t mmapSize;
	size_t mmapFlags;
	size_t mmapFd;
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
static VOID beforeRealloc(VOID* ptr,ADDRINT size, VOID * retIp,THREADID threadid)
{
	ThreadData & data = getTls(threadid);
	data.allocSize = size;
	data.allocCallsite = retIp;
	data.reallocPtr = ptr;
	//printf("malloc %lu (from %p)\n",size,retIp);
}

/*******************  FUNCTION  *********************/
static VOID afterRealloc(ADDRINT ret,THREADID threadid)
{
	//printf("    => %p\n",(void*)ret);
	ThreadData & data = getTls(threadid);
	data.tracker->onRealloc((size_t)data.allocCallsite,(size_t)data.reallocPtr,ret,data.allocSize);
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
	printf("MMAP size %lu\n",size);
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
void A_ProcessDirectCall(ADDRINT ip, ADDRINT target, ADDRINT sp)
{
	cout << "Direct call: " << Target2String(ip) << " => " << Target2String(target) << endl;
	//callStack.ProcessCall(sp, target);
}

/*******************  FUNCTION  *********************/
void A_ProcessDirectCall_ret(ADDRINT ip, ADDRINT target, ADDRINT sp)
{
	cout << "Direct call RET: " << Target2String(ip) << " <= " << Target2String(target) << endl;
	//callStack.ProcessCall(sp, target);
}

/*******************  FUNCTION  *********************/
void A_ProcessIndirectCall(ADDRINT ip, ADDRINT target, ADDRINT sp)
{
	cout << "Indirect call: " << Target2String(ip) << " => " << Target2String(target) << endl;
	//callStack.ProcessCall(sp, target);
}

/*******************  FUNCTION  *********************/
void A_ProcessIndirectCall_ret(ADDRINT ip, ADDRINT target, ADDRINT sp)
{
	cout << "Indirect call RET: " << Target2String(ip) << " <= " << Target2String(target) << endl;
	//callStack.ProcessCall(sp, target);
}

/*******************  FUNCTION  *********************/
void A_ProcessStub(ADDRINT ip, ADDRINT target, ADDRINT sp) 
{
	cout << "Instrumenting stub: " << Target2String(ip) << " => " << Target2String(target) << endl;
	cout << "STUB: ";
	cout << Target2RtnName(target) << endl;
	//callStack.ProcessCall(sp, target);
}

/*******************  FUNCTION  *********************/
void A_ProcessStub_ret(ADDRINT ip, ADDRINT target, ADDRINT sp) 
{
	cout << "Instrumenting stub RET: " << Target2String(ip) << " <= " << Target2String(target) << endl;
	cout << "STUB: ";
	cout << Target2RtnName(target) << endl;
	//callStack.ProcessCall(sp, target);
}

/*******************  FUNCTION  *********************/
static void A_ProcessReturn(ADDRINT ip, ADDRINT sp) {
	cout << "return " << Target2String(ip) <<endl;
	//callStack.ProcessReturn(sp, prevIpDoesPush);
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
					   IARG_INST_PTR, IARG_THREAD_ID,IARG_END);
		
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
					   IARG_INST_PTR, IARG_THREAD_ID,IARG_END);
		
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
	RTN mmapRtn = RTN_FindByName(img, "mmap");
	if (RTN_Valid(mmapRtn))
	{
		RTN_Open(mmapRtn);
		
		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(mmapRtn, IPOINT_AFTER, (AFUNPTR)beforeMmap,
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
static void I_Trace(TRACE trace, void *v)
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
                if( INS_IsDirectBranchOrCall(tail) ) {
                    ADDRINT target = INS_DirectBranchOrCallTargetAddress(tail);
                    INS_InsertPredicatedCall(tail, IPOINT_BEFORE,
                                             (AFUNPTR)A_ProcessDirectCall,
                                             IARG_INST_PTR,
                                             IARG_ADDRINT, target,
                                             IARG_REG_VALUE, REG_STACK_PTR,
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
	instrImageMalloc(img,v);
	instrImageRealloc(img,v);
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
	if (false)
	{
		TRACE_AddInstrumentFunction(I_Trace, 0);
		//RTN_AddInstrumentFunction(instrFunctions, 0);
	}
	PIN_AddFiniFunction(Fini, 0);

	// Register ThreadStart to be called when a thread starts and stop.
	PIN_AddThreadStartFunction(ThreadStart, NULL);
	PIN_AddThreadFiniFunction(ThreadFini, NULL);

	// Never returns
	PIN_StartProgram();
	
	return 0;
}
