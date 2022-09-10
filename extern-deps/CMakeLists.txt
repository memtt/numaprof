######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.2                        #
#            DATE     : 11/2020                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
if (NOT GTest_FOUND OR NOT GMOCK_FOUND)
	add_subdirectory(gmock-1.7.0)
endif (NOT GTest_FOUND OR NOT GMOCK_FOUND)
add_subdirectory(jsoncpp/dist)
add_subdirectory(from-htopml)
add_subdirectory(from-malt-v2)
add_subdirectory(iniparser)
