######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.3                        #
#            DATE     : 09/2022                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
if (NOT GTest_FOUND OR NOT GMOCK_FOUND)
	set(INSTALL_GTEST off)
	add_subdirectory(gmock-1.12.1)
endif (NOT GTest_FOUND OR NOT GMOCK_FOUND)
add_subdirectory(jsoncpp/dist)
add_subdirectory(from-htopml)
add_subdirectory(from-malt-v2)
add_subdirectory(iniparser)
