######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
find_package(Pintool)

######################################################
add_subdirectory(scripts)
if (PINTOOL_FOUND)
	add_subdirectory(pintool)
else(PINTOOL_FOUND)
	if(PINTOOL_PREFIX)
		message(FATAL_ERROR "You provided -DPINTOOL_PREFIX=${PINTOOL_PREFIX} but pin is not found in this directory ! Disable it or provide valid path.")
	endif(PINTOOL_PREFIX)
endif (PINTOOL_FOUND)
