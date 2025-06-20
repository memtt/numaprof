######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.6                        #
#            DATE     : 06/2025                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
add_library(numaprof-portability OBJECT NumaTopo.cpp OS.cpp Clock.cpp)
SET_TARGET_PROPERTIES(numaprof-portability PROPERTIES COMPILE_FLAGS -fPIC)

######################################################
#install procedure
#INSTALL(FILES ${JSON_HEADERS} DESTINATION include/htopml)
