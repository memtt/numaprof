######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#Setup paths to gtest/gmock headers and library
MACRO(setup_internal_gmock_and_gtest)
	if(GTest_FOUND AND GMOCK_FOUND)
		set(GMOCK_SOURCE_DIR /usr)
		set(GMOCK_INCLUDE_DIR ${GMOCK_SOURCE_DIR}/include)
		set(GMOCK_INCLUDE_DIRS ${GMOCK_SOURCE_DIR}/include)
		set(GMOCK_BOTH_LIBRARIES ${GTEST_BOTH_LIBRARIES})
	else(GTest_FOUND AND GMOCK_FOUND)
		set(GMOCK_SOURCE_DIR ${CMAKE_SOURCE_DIR}/extern-deps/gmock-1.12.1)
		set(GMOCK_INCLUDE_DIR ${GMOCK_SOURCE_DIR}/googlemock/include)
		set(GMOCK_INCLUDE_DIRS ${GMOCK_SOURCE_DIR}/googlemock/include)
		set(GMOCK_BOTH_LIBRARIES gmock gmock_main)
		set(GTEST_BOTH_LIBRARIES gtest)
		set(GTEST_INCLUDE_DIR ${GMOCK_SOURCE_DIR}/googletest/include/)
		set(GTEST_INCLUDE_DIRS ${GMOCK_SOURCE_DIR}/googletest/include/)
	endif(GTest_FOUND AND GMOCK_FOUND)
ENDMACRO(setup_internal_gmock_and_gtest)

######################################################
#Short macro to quicly declare some unit tests
MACRO(declare_test test_name)
	add_executable(${test_name} ${test_name}.cpp)
	target_link_libraries(${test_name} ${GTEST_BOTH_LIBRARIES} ${GMOCK_BOTH_LIBRARIES} ${EXTRA_LIBS})
	add_test(${test_name} ${test_name})
ENDMACRO(declare_test)

######################################################
MACRO(numaprof_enable_gcc_coverage)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -fprofile-arcs -ftest-coverage")
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O0 -fprofile-arcs -ftest-coverage")
	set(CMAKE_EXE_LINKER_FLAGS_FLAGS "${CMAKE_C_FLAGS} -O0 -fprofile-arcs -ftest-coverage")
ENDMACRO(numaprof_enable_gcc_coverage)

######################################################
MACRO(numaprof_enable_cxx_11)
	include(CheckCXXCompilerFlag)
	CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
	if(COMPILER_SUPPORTS_CXX11)
		message(STATUS "Using -std=c++11")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	else()
			message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
	endif()
ENDMACRO(numaprof_enable_cxx_11)
