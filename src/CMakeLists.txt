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
if (Qt5Widgets_FOUND AND Qt5WebEngineWidgets_FOUND AND Qt5Network_FOUND)
	set(QT5_FOUND OK)
	message(STATUS "Find QT5 to build view")
else (Qt5Widgets_FOUND AND Qt5WebEngineWidgets_FOUND AND Qt5Network_FOUND)
	message(STATUS "QT5 webkit not found, skip QT5 view")
endif (Qt5Widgets_FOUND AND Qt5WebEngineWidgets_FOUND AND Qt5Network_FOUND)

######################################################
add_subdirectory(lib)
add_subdirectory(manpages)
add_subdirectory(testcase)
add_subdirectory(integration)
if (ENABLE_WEBVIEW)
	add_subdirectory(webview)
	if (QT5_FOUND)
		add_subdirectory(qtview)
	endif (QT5_FOUND)
endif (ENABLE_WEBVIEW)
