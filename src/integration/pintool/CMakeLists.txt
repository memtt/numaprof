######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(PINTOOL_CFLAGS "-O3 -g")
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(PINTOOL_CFLAGS "-O3 -DNDEBUG")
endif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")

######################################################
# fix required for ubuntu 18....
# Hum no, it compiles but fail to launc due to missing symbol
#CHECK_CXX_COMPILER_FLAG("-faligned-new" HAVE_GCC_ALIGNED_NEW)
#if (HAVE_GCC_ALIGNED_NEW)
#	message(STATUS "Enable usage of -faligned-new for Pintool integration")
#	set(GCC_ALIGNED_NEW_FLAG "-faligned-new")
#endif (HAVE_GCC_ALIGNED_NEW)

######################################################
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Makefile.pin.in ${CMAKE_CURRENT_BINARY_DIR}/Makefile.pin @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/makefile.rules.in ${CMAKE_CURRENT_BINARY_DIR}/makefile.rules @ONLY)

######################################################
file(GLOB PIN_MODULE_SOURCES LIST_DIRECTORIES false ${CMAKE_SOURCE_DIR}/src/lib/common/*.cpp
                                                    ${CMAKE_SOURCE_DIR}/src/lib/common/*.hpp
                                                    ${CMAKE_SOURCE_DIR}/src/lib/core/*.hpp
                                                    ${CMAKE_SOURCE_DIR}/src/lib/core/*.cpp
                                                    ${CMAKE_SOURCE_DIR}/src/lib/portability/*.hpp
                                                    ${CMAKE_SOURCE_DIR}/src/lib/portability/*.cpp
                                                    ${CMAKE_CURRENT_SOURCE_DIR}/numaprof.cpp
                                                    ${CMAKE_SOURCE_DIR}/extern-deps/iniparser/src/*.cpp
                                                    ${CMAKE_SOURCE_DIR}/extern-deps/iniparser/src/*.hpp
                                                    ${CMAKE_SOURCE_DIR}/extern-deps/from-numactl/*.cpp
                                                    ${CMAKE_SOURCE_DIR}/extern-deps/from-numactl/*.hpp
                                                    ${CMAKE_SOURCE_DIR}/extern-deps/from-htopml/*.cpp
                                                    ${CMAKE_SOURCE_DIR}/extern-deps/from-htopml/*.hpp
                                                    ${CMAKE_SOURCE_DIR}/extern-deps/from-malt-v2/*.cpp
                                                    ${CMAKE_SOURCE_DIR}/extern-deps/from-malt-v2/*.hpp)

######################################################
#OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/obj-intel64/libnumaprof-pintool.so
add_custom_target(pintool ALL COMMAND make -f Makefile.pin
                              COMMENT "Building pintool plugin..."
                              WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                              DEPENDS ${PIN_MODULE_SOURCES})

######################################################
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/obj-intel64/libnumaprof-pintool.so DESTINATION lib)
