######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
add_subdirectory(lib)
add_subdirectory(manpages)
add_subdirectory(testcase)
add_subdirectory(integration)
if (ENABLE_WEBVIEW)
	add_subdirectory(webview)
endif (ENABLE_WEBVIEW)
