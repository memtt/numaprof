######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
add_library(numaprof SHARED PageTable.cpp
							TopoHwloc.cpp
							Debug.cpp
							Helper.cpp
							NumaTopo.cpp
							../extern-deps/from-numactl/MovePages.cpp
							ThreadTracker.cpp
							ProcessTracker.cpp
							Stats.cpp
							MallocTracker.cpp
							$<TARGET_OBJECTS:htopml-json>)
target_link_libraries(numaprof ${HWLOC_LIBRARY})
