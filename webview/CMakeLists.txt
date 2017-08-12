######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
set(WEBVIEW_PATH ${CMAKE_INSTALL_PREFIX}/share/numaprof/webview)
configure_file(numaprof-webview.sh.in numaprof-webview @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/numaprof-webview DESTINATION bin
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

######################################################
if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bower_components/ OR NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/deps)
	add_custom_command (OUTPUT deps-loaded
					COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/prepare.sh ${CMAKE_CURRENT_BINARY_DIR}
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
	add_custom_target(run ALL DEPENDS deps-loaded)
	INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/deps DESTINATION share/numaprof/webview)
	INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bower_components DESTINATION share/numaprof/webview)
else (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bower_components/ OR NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/deps)
	INSTALL(DIRECTORY ${CMAKE_SOURCE_BINARY_DIR}/deps DESTINATION share/numaprof/webview PATTERN "*.pyc" EXCLUDE)
	INSTALL(DIRECTORY ${CMAKE_SOURCE_BINARY_DIR}/bower_components DESTINATION share/numaprof/webview)
endif (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bower_components/ OR NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/deps)

######################################################
# Install the webview into share directory
INSTALL(FILES Helper.py nocache.py ProfileHandler.py server.py DESTINATION share/numaprof/webview)
INSTALL(DIRECTORY templates DESTINATION share/numaprof/webview)
INSTALL(DIRECTORY static DESTINATION share/numaprof/webview)
