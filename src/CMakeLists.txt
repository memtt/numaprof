######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
# Find the QtWidgets library
find_package(Qt5Widgets QUIET)
find_package(Qt5WebEngineWidgets QUIET)
find_package(Qt5WebKitWidgets QUIET)
find_package(Qt5Network QUIET)

######################################################
if (Qt5WebEngineWidgets_FOUND OR Qt5WebKitWidgets_FOUND)
	set(Qt5Webkit_FOUND OK)
endif (Qt5WebEngineWidgets_FOUND OR Qt5WebKitWidgets_FOUND)

if (Qt5Widgets_FOUND AND Qt5Webkit_FOUND AND Qt5Network_FOUND)
	set(QT5_FOUND OK)
	message(STATUS "Find QT5 to build view")
else (Qt5Widgets_FOUND AND Qt5Webkit_FOUND AND Qt5Network_FOUND)
	message(STATUS "QT5 webkit not found, skip QT5 view")
endif (Qt5Widgets_FOUND AND Qt5Webkit_FOUND AND Qt5Network_FOUND)

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
