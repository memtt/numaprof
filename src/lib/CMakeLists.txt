######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
find_package(Numactl REQUIRED)
include_directories(${NUMACTL_INCLUDE_DIR})
include_directories(../../extern-deps/iniparser/src)

######################################################
add_library(numaprof SHARED $<TARGET_OBJECTS:numaprof-core>
							$<TARGET_OBJECTS:numaprof-portability>
							$<TARGET_OBJECTS:numaprof-common>
							$<TARGET_OBJECTS:htopml-json>
							$<TARGET_OBJECTS:malt-core>
							$<TARGET_OBJECTS:iniparser>)
target_link_libraries(numaprof ${NUMACTL_LIBRARY})

######################################################
add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(portability)

######################################################
#target_link_libraries(numaprof ${HWLOC_LIBRARY})
INSTALL(TARGETS numaprof
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib)

######################################################
add_subdirectory(tests)