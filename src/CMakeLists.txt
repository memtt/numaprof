######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
# Find the QtWidgets library
find_package(Qt5Widgets QUIET)
find_package(Qt5WebEngineWidgets QUIET)
find_package(Qt5Network QUIET)

######################################################
add_subdirectory(lib)
add_subdirectory(manpages)
add_subdirectory(testcase)
add_subdirectory(integration)
if (ENABLE_WEBVIEW)
	add_subdirectory(webview)
	if (Qt5Widgets_FOUND AND Qt5WebEngineWidgets_FOUND)
		add_subdirectory(qtview)
	endif (Qt5Widgets_FOUND AND Qt5WebEngineWidgets_FOUND)
endif (ENABLE_WEBVIEW)
