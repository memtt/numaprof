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
 * Hybrid Binary Rewriting for Memory Access Instrumentation: http://doi.acm.org/10.1145/1952682.1952711
 * Analyzing Parallel Programs with Pin, IEEE Computer. March 2010, vol. 43, no. 3, pages 34-41.
 * Scalable Support for Multithreaded Applications on Dynamic Binary Instrumentation Systems," Proceedings of the 2009 International Symposium on Memory Management (ISMM). Dublin, Ireland. June 2009, pages 20-29.
 * https://software.intel.com/en-us/articles/pin-a-binary-instrumentation-tool-papers
 * Profiling Directed NUMA Optimization on Linux Systems: A Case Study of the Gaussian Computational Chemistry Code : http://ieeexplore.ieee.org/document/6012912/?reload=true
 * https://eurodw17.kaust.edu.sa/abstracts/eurodw17-final8.pdf
 * http://graal.ens-lyon.fr/~sdelamar/data/jcalc_lmorel.pdf
 * http://queue.acm.org/detail.cfm?id=2852078
 * https://tel.archives-ouvertes.fr/tel-01253537
 * https://www.researchgate.net/publication/261322796_Call_Paths_for_Pin_Tools
 * https://github.com/chabbimilind/cctlib