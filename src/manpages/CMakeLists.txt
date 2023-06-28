######################################################
#            PROJECT  : MALT                         #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
#            AUTHOR   : Valat Sébastien              #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#Check that files are updated
# if (${CMAKE_CURRENT_SOURCE_DIR}/malt.ronn IS_NEWER_THAN ${CMAKE_CURRENT_SOURCE_DIR}/malt.1.bz2)
# 	message(WARNING "Please run make command into ${CMAKE_CURRENT_SOURCE_DIR} to update the manpages compiled with ronn tool")
# endif (${CMAKE_CURRENT_SOURCE_DIR}/malt.ronn IS_NEWER_THAN ${CMAKE_CURRENT_SOURCE_DIR}/malt.1.bz2)
# 
# #Check that files are updated
# if (${CMAKE_CURRENT_SOURCE_DIR}/malt-passwd.ronn IS_NEWER_THAN ${CMAKE_CURRENT_SOURCE_DIR}/malt-passwd.1.bz2)
# 	message(WARNING "Please run make command into ${CMAKE_CURRENT_SOURCE_DIR} to update the manpages compiled with ronn tool")
# endif (${CMAKE_CURRENT_SOURCE_DIR}/malt-passwd.ronn IS_NEWER_THAN ${CMAKE_CURRENT_SOURCE_DIR}/malt-passwd.1.bz2)
# 
# #Check that files are updated
# if (${CMAKE_CURRENT_SOURCE_DIR}/malt-webserver.ronn IS_NEWER_THAN ${CMAKE_CURRENT_SOURCE_DIR}/malt-webserver.1.bz2)
# 	message(WARNING "Please run make command into ${CMAKE_CURRENT_SOURCE_DIR} to update the manpages compiled with ronn tool")
# endif (${CMAKE_CURRENT_SOURCE_DIR}/malt-webserver.ronn IS_NEWER_THAN ${CMAKE_CURRENT_SOURCE_DIR}/malt-webserver.1.bz2)

######################################################
#install
install(FILES numaprof.1.bz2 numaprof-passwd.1.bz2 numaprof-webview.1.bz2
        DESTINATION share/man/man1)

#optional
if (QT5_FOUND)
	install(FILES numaprof-qt5.1.bz2
            DESTINATION share/man/man1)
endif (QT5_FOUND)
