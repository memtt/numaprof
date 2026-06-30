######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.7                        #
#            DATE     : 06/2026                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
add_library(numaprof-core OBJECT MallocTracker.cpp PageTable.cpp ProcessTracker.cpp Stack.cpp Stats.cpp ThreadTracker.cpp AccessMatrix.cpp)
SET_TARGET_PROPERTIES(numaprof-core PROPERTIES COMPILE_FLAGS -fPIC)

######################################################
#install procedure
#INSTALL(FILES ${JSON_HEADERS} DESTINATION include/htopml)
