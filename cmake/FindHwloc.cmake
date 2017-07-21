######################################################
# - Try to find hwloc (https://github.com/open-source-parsers/hwloc/)
# Once done this will define
#  HWLOC_FOUND - System has hwloc
#  HWLOC_INCLUDE_DIRS - The hwloc include directories
#  HWLOC_LIBRARIES - The libraries needed to use hwloc
#  HWLOC_DEFINITIONS - Compiler switches required for using hwloc

######################################################
set(HWLOC_PREFIX "" CACHE STRING "Help cmake to find hwloc library (https://www.open-mpi.org/projects/hwloc/) into your system.")

######################################################
find_path(HWLOC_INCLUDE_DIR hwloc.h
	HINTS ${HWLOC_PREFIX}/include
	PATH_SUFFIXES hwloc)

######################################################
find_library(HWLOC_LIBRARY NAMES hwloc
	HINTS ${HWLOC_PREFIX}/lib)

######################################################
set(HWLOC_LIBRARIES ${HWLOC_LIBRARY} )
set(HWLOC_INCLUDE_DIRS ${HWLOC_INCLUDE_DIR} )

######################################################
include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set HWLOC_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Hwloc  DEFAULT_MSG
	HWLOC_LIBRARY HWLOC_INCLUDE_DIR)

######################################################
mark_as_advanced(HWLOC_INCLUDE_DIR HWLOC_LIBRARY )
