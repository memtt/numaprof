######################################################
#            PROJECT  : htopml                       #
#            VERSION  : 0.0.0                        #
#            DATE     : 02/2015                      #
#            AUTHOR   : Valat Sébastien              #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
add_library(malt-core OBJECT Debug.cpp FormattedMessage.cpp LinuxProcMapReader.cpp SymbolRegistry.cpp)
SET_TARGET_PROPERTIES(malt-core PROPERTIES COMPILE_FLAGS -fPIC)

######################################################
#install procedure
#INSTALL(FILES ${JSON_HEADERS} DESTINATION include/htopml)