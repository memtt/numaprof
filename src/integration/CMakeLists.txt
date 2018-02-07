######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.0-dev                    #
#            DATE     : 02/2018                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
find_package(Pintool)

######################################################
add_subdirectory(scripts)
if (PINTOOL_FOUND)
	add_subdirectory(pintool)
endif (PINTOOL_FOUND)
