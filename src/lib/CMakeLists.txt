######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
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
							$<TARGET_OBJECTS:numaprof-caches>
							$<TARGET_OBJECTS:htopml-json>
							$<TARGET_OBJECTS:malt-core>
							$<TARGET_OBJECTS:iniparser>)
target_link_libraries(numaprof ${NUMACTL_LIBRARY})
target_link_libraries(numaprof dl)
set_target_properties(numaprof PROPERTIES VERSION 1.1.5 SOVERSION 1)

######################################################
add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(portability)
add_subdirectory(caches)

######################################################
#target_link_libraries(numaprof ${HWLOC_LIBRARY})
INSTALL(TARGETS numaprof
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib)

######################################################
if (ENABLE_TESTS)
	add_subdirectory(tests)
endif (ENABLE_TESTS)
