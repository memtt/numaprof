Numaprof is currently a prototype to implement a NUMA memory 
profiler. The idea is to instrument the read/write operations in 
the application and check the NUMA location of the thread at 
accesss time to compare it to the memory location of the data.

I'm currently testing an implementation based on pintool but I'm 
also looking to possibly use a CLANG/LLVM plugin which offer
the same thing but without support on non recompiled code :
https://llvm.org/svn/llvm-project/safecode/trunk/lib/CommonMemorySafety/InstrumentMemoryAccesses.cpp
