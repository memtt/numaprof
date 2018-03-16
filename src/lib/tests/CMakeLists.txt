######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.0-dev                    #
#            DATE     : 02/2018                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#Use gtest
include_directories(${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})
include_directories(../)

######################################################
#add_definitions(-DTEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data")

######################################################
set(TEST_NAMES TestPageTable TestHelper TestMovePages TestStack TestMallocTracker TestFormattedMessage TestStaticAssoCache)

######################################################
FOREACH(test_name ${TEST_NAMES})
	add_executable(${test_name} ${test_name}.cpp)
	target_link_libraries(${test_name} ${GTEST_BOTH_LIBRARIES} ${GMOCK_BOTH_LIBRARIES} numaprof)
	daq_add_test(${test_name} ${test_name})
ENDFOREACH(test_name)

######################################################
add_executable(ReplayAllocTracker ReplayAllocTracker.cpp)
target_link_libraries(ReplayAllocTracker numaprof)
