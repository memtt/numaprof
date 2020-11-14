######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.2                        #
#            DATE     : 11/2020                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#Usage of OBJECT keyword in add_library require 2.8.8 (or something like that, need to check)
#QT5 requires 2.8.3 mini, recommanded 3.1.0
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
        numaprof_enable_gcc_coverage()
endif(ENABLE_GCC_COVERAGE)

######################################################
numaprof_enable_cxx_11()

######################################################
add_subdirectory(src)
add_subdirectory(extern-deps)
