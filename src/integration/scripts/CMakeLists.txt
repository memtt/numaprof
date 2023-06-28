######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#pintool
if (PINTOOL_FOUND)
	#build
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/numaprof-pintool.sh.in ${CMAKE_CURRENT_BINARY_DIR}/numaprof-pintool @ONLY)

	#install
	INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/numaprof-pintool DESTINATION bin/)
	INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/numaprof-pintool DESTINATION bin RENAME numaprof)
endif (PINTOOL_FOUND)

######################################################
#install
INSTALL(PROGRAMS numaprof-to-callgrind.py DESTINATION bin RENAME numaprof-to-callgrind)
