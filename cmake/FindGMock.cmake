######################################################
# - Try to find GMOCK (https://github.com/google/googletest)
# Once done this will define
#  GMOCK_FOUND - System has GMock
#  GMOCK_INCLUDE_DIRS - The GMock include directories
#  GMOCK_LIBRARIES - The libraries needed to use GMock
#  GMOCK_DEFINITIONS - Compiler switches required for using GMock

######################################################
set(GMOCK_PREFIX "" CACHE STRING "Help cmake to find GMock library (https://github.com/google/googletest) into your system.")

######################################################
find_path(GMOCK_INCLUDE_DIR gmock.h
	HINTS ${GMOCK_PREFIX}/include
	PATH_SUFFIXES gmock)

######################################################
set(GMOCK_INCLUDE_DIRS ${GMOCK_INCLUDE_DIR} )

######################################################
include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set GMOCK_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(GMock DEFAULT_MSG GMOCK_INCLUDE_DIR)

######################################################
mark_as_advanced(GMOCK_INCLUDE_DIR)
