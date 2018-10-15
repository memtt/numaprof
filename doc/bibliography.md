Biblio
======

Tools/Libs:
-----------

 * DynamoRIO: http://www.dynamorio.org/
 * Tabarnac: https://github.com/dbeniamine/Tabarnac
 * CCTLib: https://github.com/chabbimilind/cctlib
 * NumaTop: https://01.org/sites/default/files/documentation/numatop_introduction_0.pdf
 * SnPerf: http://www.cs.utah.edu/~uros/snperf
 * HPCToolkit: https://github.com/HPCToolkit/hpctoolkit
 * Tabarnac: https://gitlab.com/dbeniamine/Tabarnac
 * SIMT
 * Memaxis
 * Memphis
 * MALT: https://memtt.github.io/
 * Pin: https://software.intel.com/en-us/articles/pin-a-dynamic-binary-instrumentation-tool
 * HwLoc: https://www.open-mpi.org/projects/hwloc/
 * GoogleTest: https://github.com/google/googletest
 * Another NUMAPROF (not availble, just named into the article): https://link.springer.com/chapter/10.1007/978-3-319-27140-8_6
 * NumaMMA : https://github.com/numamma/numamma
 * Numap: https://github.com/numap-library/numap

Slides:
-------

 * NUMA Profiling for Dynamic Dataflow Applications: https://graal.ens-lyon.fr/~sdelamar/data/jcalc_lmorel.pdf
 * A Data-centric Profiler for Parallel Programs: http://www.paradyn.org/CSCADS2013/slides/liu13.pdf
 * Scientific approaches: NUMA Profilers/analyzing runtime behavior Non-Uniform Memory Access (NUMA) Seminar: https://www.dcl.hpi.uni-potsdam.de/teaching/numasem/slides/NUMASem_Profiler.pdf
 
Papers/web articles:
---------------------

 * A tool to analyze the performance of multithreaded programs on NUMA architectures: https://doi.org/10.1145/2692916.2555271
 * Using Valgrind to detect undefined value errors with bit-precision: http://valgrind.org/docs/memcheck2005.pdf
 * MemProf: a Memory Profiler for NUMA Multicore Systems: https://www.usenix.org/system/files/conference/atc12/atc12-final229.pdf
 * Dissecting On-Node Memory Access Performance: A Semantic Approach: http://charm.cs.illinois.edu/~bhatele/pubs/pdf/2014/sc2014c.pdf
 * The ccNUMA Memory Profiler: http://www.cs.utah.edu/~ald/pubs/CC-numa.pdf
 * Hybrid Binary Rewriting for Memory Access Instrumentation: http://doi.acm.org/10.1145/1952682.1952711
 * Analyzing Parallel Programs with Pin: http://doi.org/10.1109/MC.2010.60
 * Scalable Support for Multithreaded Applications on Dynamic Binary Instrumentation Systems: http://doi.org/10.1145/1542431.1542435
 * PinTool paper list: https://software.intel.com/en-us/articles/pin-a-binary-instrumentation-tool-papers
 * Profiling Directed NUMA Optimization on Linux Systems: A Case Study of the Gaussian Computational Chemistry Code : https://doi.org/10.1109/IPDPS.2011.100
 * Improving performance on NUMA systems: https://tel.archives-ouvertes.fr/tel-01549294
 * Profiling for Asymmetric NUMA Systems: https://eurodw17.kaust.edu.sa/abstracts/eurodw17-final8.pdf
 * Challenges of Memory Management on Modern NUMA System: https://queue.acm.org/detail.cfm?id=2852078
 * Contribution à l'amélioration des méthodes d'optimisation de la gestion de la mémoire dans le cadre du Calcul Haute Performance: https://tel.archives-ouvertes.fr/tel-01253537
 * Call Paths for Pin Tools: https://www.researchgate.net/publication/261322796_Call_Paths_for_Pin_Tools
 * Memphis: Finding and fixing NUMA-related performance problems on multi-core platforms: https://doi.org/10.1109/ISPASS.2010.5452060
 * http://doi.acm.org/10.1145/1806651.1806667
 * TABARNAC: Tools for Analyzing Behavior of Applications Running on NUMA Architecture: https://hal.inria.fr/hal-01202105
 * Characterizing communication and page usage of parallel applications for thread and data mapping : https://doi.org/10.1016/j.peva.2015.03.001 
 * Dissecting On-Node Memory Access Performance: A Semantic Approach: https://doi.org/10.1109/SC.2014.19
 * SIGMA: A Simulator Infrastructure to Guide Memory Analysis: https://doi.org/10.1109/SC.2002.10055
 * Visualizing the Memory Access Behavior of Shared Memory Applications on NUMA Architectures: https://doi.org/10.1007/3-540-45718-6_91
 * SIMT/OMP: A Toolset to Study and Exploit Memory Locality of OpenMP Applications on NUMA Architectures : https://doi.org/10.1007/978-3-540-31832-3_5
 * A Flexible Data Model to Support Multi-domain Performance Analysis: https://doi.org/10.1007/978-3-319-16012-2_10
 * MALT, A MALloc Tracker: https://doi.org/10.1145/3141865.3141867
 * A Data-Centric Tool to Improve the Performance of Multithreaded Program on NUMA: https://doi.org/10.1007/978-3-319-27140-8_6
 * NumaMMA: NUMA MeMory Analyzer: https://doi.org/10.1145/3225058.3225094
 * numap: A Portable Library For Low Level Memory Profiling: https://hal.inria.fr/hal-01285522

Technical help:
---------------

 * Dangers of using dlsym() with RTLD_NEXT: http://optumsoft.com/dangers-of-using-dlsym-with-rtld_next/
 * The pintool is broken: https://github.com/wapiflapi/villoc/issues/3
 * RedHard MPOL_PREFERED kernel bug report: https://access.redhat.com/solutions/3155591
 * Man move_pages() : http://man7.org/linux/man-pages/man2/move_pages.2.html
 * Man sched_getaffinity() : https://linux.die.net/man/2/sched_getaffinity
 * Man mbind(): http://man7.org/linux/man-pages/man2/mbind.2.html
 * Man numa: http://man7.org/linux/man-pages/man3/numa.3.html

 Looking for python + qt webkit to build gui
 -------------------------------------------
 
 In python :
  * https://pawelmhm.github.io/python/pyqt/qt/webkit/2015/09/08/browser.html
  * https://github.com/agateau/pyqt-webkit-tutorial/blob/master/tut1/tut1.markdown
  * https://wiki.python.org/moin/PyQt/Using%20a%20Custom%20Protocol%20with%20QtWebKit
  * https://riverbankcomputing.com/pipermail/pyqt/2016-September/038076.html
  

Docs on cache replacement policy for cache simulation
-----------------------------------------------------

 * http://www.csbio.unc.edu/mcmillan/Media/L20Spring2013.pdf
 * https://stackoverflow.com/questions/23448528/how-is-an-lru-cache-implemented-in-a-cpu
 * https://www.powershow.com/view/95163-NzkyO/4_4_20Page_20replacement_20algorithms_powerpoint_ppt_presentation
 * https://www.cs.swarthmore.edu/~margarel/Papers/CS25.pdf
 * http://blog.stuffedcow.net/2013/01/ivb-cache-replacement/