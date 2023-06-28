######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
if (NOT IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bower_components)
	find_package(Npm REQUIRED)
endif()
if (NOT IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps)
	find_package(Pip REQUIRED)
endif()

######################################################
set(WEBVIEW_PATH ${CMAKE_INSTALL_PREFIX}/share/numaprof/webview)
configure_file(webview.sh.in numaprof-webview @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/numaprof-webview DESTINATION bin)
configure_file(webview-htpasswd.sh.in numaprof-passwd @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/numaprof-passwd DESTINATION bin)

######################################################
if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bower_components/ OR NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/deps)
	add_custom_command (OUTPUT deps-loaded
					COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/prepare.sh ${CMAKE_CURRENT_BINARY_DIR}
					COMMENT "Fetching webview python/javascript dependencies..."
					DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/prepare.sh
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
	add_custom_target(run ALL DEPENDS deps-loaded)
	INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/deps DESTINATION share/numaprof/webview)
	INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bower_components DESTINATION share/numaprof/webview)
else (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bower_components/ OR NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/deps)
	INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps DESTINATION share/numaprof/webview PATTERN "*.pyc" EXCLUDE)
	INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bower_components DESTINATION share/numaprof/webview)
endif (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bower_components/ OR NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/deps)

######################################################
# Install the webview into share directory
INSTALL(FILES Helper.py nocache.py ProfileHandler.py server.py nhtpasswd.py DESTINATION share/numaprof/webview)
INSTALL(DIRECTORY templates DESTINATION share/numaprof/webview)
INSTALL(DIRECTORY static DESTINATION share/numaprof/webview)
