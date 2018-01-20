######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#Usage of OBJECT keyword in add_library require 2.8.8 (or something like that, need to check)
cmake_minimum_required(VERSION 2.8.8)
project(numaprof)

######################################################
#global setup
enable_testing()

######################################################
#Add internal search path for scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)

######################################################
#Delegate some cmake scripts
include(${CMAKE_SOURCE_DIR}/cmake/macros.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/unittest-macros.cmake)

######################################################
set(ENABLE_WEBVIEW "ON" CACHE BOOL "Enable installation of webview")

######################################################
#Some global setup
setup_internal_gmock_and_gtest()
if (ENABLE_GCC_COVERAGE)
        daq_enable_gcc_coverage()
endif(ENABLE_GCC_COVERAGE)

######################################################
numaprof_enable_cxx_11()

######################################################
#find_package(Hwloc REQUIRED)
include_directories(${HWLOC_INCLUDE_DIRS})

######################################################
add_subdirectory(src)
add_subdirectory(extern-deps)
add_subdirectory(tests)
add_subdirectory(manpages)
add_subdirectory(testcase)
add_subdirectory(integration)
if (ENABLE_WEBVIEW)
	add_subdirectory(webview)
endif (ENABLE_WEBVIEW)
