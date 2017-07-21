/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */

#include "pin.H"
#include <cstdio>
#include <MovePages.hpp>

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


int id = 0;
int cnt = 1;

FILE * trace;

// Print a memory read record
static VOID RecordMemRead(VOID * ip, VOID * addr)
{
	if (id == 0)
	{
		id = cnt;
		cnt++;
	}
	fprintf(trace,"%p: R %p, thread %d NUMA %d\n", ip, addr,id,getNumaOfPage(addr));
}

// Print a memory write record
static VOID RecordMemWrite(VOID * ip, VOID * addr)
{
    if (id == 0)
	{
		id = cnt;
		cnt++;
	}
	fprintf(trace,"%p: W %p, thread %d NUMA %d\n", ip, addr,id,getNumaOfPage(addr));
}


/*******************  FUNCTION  *********************/
static VOID beforeMalloc(ADDRINT size)
{
	printf("malloc %lu\n",size);
}

/*******************  FUNCTION  *********************/
static VOID afterMalloc(ADDRINT ret)
{
	printf("    => %p\n",(void*)ret);
}

/*******************  FUNCTION  *********************/
static VOID beforeCalloc(ADDRINT nmemb,ADDRINT size)
{
	printf("calloc %lu %lu\n",nmemb,size);
}

/*******************  FUNCTION  *********************/
static VOID beforeFree(ADDRINT ptr)
{
	printf("free %p\n",(void*)ptr);
}

/*******************  FUNCTION  *********************/
void beforeFuncPrint(void * fctAddr)
{
	printf("Enter in %p\n",fctAddr);
}

/*******************  FUNCTION  *********************/
void afterFunc(void * fctAddr)
{
    printf("Exit %p\n",fctAddr);
}

/*******************  FUNCTION  *********************/
VOID instrImageMalloc(IMG img, VOID *v)
{
	// Instrument the malloc() and free() functions.  Print the input argument
	// of each malloc() or free(), and the return value of malloc().
	//
	//  Find the malloc() function.
    printf("search %s\n",MALLOC);
	RTN mallocRtn = RTN_FindByName(img, MALLOC);
	if (RTN_Valid(mallocRtn))
    {
        printf("find malloc\n");
		RTN_Open(mallocRtn);
		
		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(mallocRtn, IPOINT_BEFORE, (AFUNPTR)beforeMalloc,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
		               IARG_END);
		
		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(mallocRtn, IPOINT_AFTER, (AFUNPTR)afterMalloc,
					   IARG_FUNCRET_EXITPOINT_VALUE,
		               IARG_END);
		RTN_Close(mallocRtn);
    }
}

/*******************  FUNCTION  *********************/
VOID instrImageCalloc(IMG img, VOID *v)
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
		               IARG_END);

		// Instrument malloc() to print the input argument value and the return value.
		RTN_InsertCall(callocRtn, IPOINT_AFTER, (AFUNPTR)afterMalloc,
					   IARG_FUNCRET_EXITPOINT_VALUE,
		               IARG_END);
		RTN_Close(callocRtn);
    }
}

/*******************  FUNCTION  *********************/
VOID instrImageFree(IMG img, VOID *v)
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
		               IARG_END);
		RTN_Close(freeRtn);
    }
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
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
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_END);
        }
    }
}

/*******************  FUNCTION  *********************/
//Inspirate from code exemple of pintool doc
VOID instrImage(IMG img, VOID *v)
{
	instrImageMalloc(img,v);
	instrImageCalloc(img,v);
	instrImageFree(img,v);
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
 		RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)beforeFuncPrint, IARG_PTR, addr, IARG_END);
 		RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)afterFunc, IARG_PTR, addr, IARG_END);
		//gblState.names.setupNewEntry(addr,RTN_Name(rtn));
 	}

	RTN_Close(rtn);
}

VOID Fini(INT32 code, VOID *v)
{
    fprintf(trace, "#eof\n");
    fclose(trace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage()
{
    PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return Usage();

    trace = fopen("pinatrace.out", "w");

    //IMG_AddInstrumentFunction(instrImage, 0);
    IMG_AddInstrumentFunction(instrImageMalloc, 0);
    INS_AddInstrumentFunction(Instruction, 0);
    RTN_AddInstrumentFunction(instrFunctions, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    
    return 0;
}
