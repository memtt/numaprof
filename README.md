Numaprof is currently a prototype to implement a NUMA memory 
profiler. The idea is to instrument the read/write operations in 
the application and check the NUMA location of the thread at 
accesss time to compare it to the memory location of the data.

I'm currently testing an implementation based on pintool but I'm 
also looking to possibly use a CLANG/LLVM plugin which offer
the same thing but without support on non recompiled code :
https://llvm.org/svn/llvm-project/safecode/trunk/lib/CommonMemorySafety/InstrumentMemoryAccesses.cpp

Biblio:
 * http://valgrind.org/docs/memcheck2005.pdf
 * https://www.usenix.org/system/files/conference/atc12/atc12-final229.pdf
 * https://www.dcl.hpi.uni-potsdam.de/teaching/numasem/slides/NUMASem_Profiler.pdf
 * https://tel.archives-ouvertes.fr/tel-01549294/document
 * http://charm.cs.illinois.edu/~bhatele/pubs/pdf/2014/sc2014c.pdf
 * http://www.cs.utah.edu/~ald/pubs/CC-numa.pdf
 * http://www.cs.utah.edu/~uros/snperf
 * https://01.org/sites/default/files/documentation/numatop_introduction_0.pdf
 * http://www.paradyn.org/CSCADS2013/slides/liu13.pdf