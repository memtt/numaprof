######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.6                        #
#            DATE     : 06/2025                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
add_library(numaprof-common OBJECT Debug.cpp Helper.cpp Options.cpp FormattedMessage.cpp)
SET_TARGET_PROPERTIES(numaprof-common PROPERTIES COMPILE_FLAGS -fPIC)

######################################################
#install procedure
#INSTALL(FILES ${JSON_HEADERS} DESTINATION include/htopml)
