######################################################
# - Try to find numactl (https://github.com/numactl/numactl)
# Once done this will define
#  NUMACTL_FOUND - System has numactl
#  NUMACTL_INCLUDE_DIRS - The numactl include directories
#  NUMACTL_LIBRARIES - The libraries needed to use numactl
#  NUMACTL_DEFINITIONS - Compiler switches required for using numactl

######################################################
set(NUMACTL_PREFIX "" CACHE STRING "Help cmake to find numactl/libnuma library (https://github.com/numactl/numactl) into your system.")

######################################################
find_path(NUMACTL_INCLUDE_DIR numa.h
	HINTS ${NUMACTL_PREFIX}/include)

######################################################
find_library(NUMACTL_LIBRARY NAMES numa
	HINTS ${NUMACTL_PREFIX}/lib)

######################################################
set(NUMACTL_LIBRARIES ${NUMACTL_LIBRARY} )
set(NUMACTL_INCLUDE_DIRS ${NUMACTL_INCLUDE_DIR} )

######################################################
include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set NUMACTL_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Numactl  DEFAULT_MSG
	NUMACTL_LIBRARY NUMACTL_INCLUDE_DIR)

######################################################
mark_as_advanced(NUMACTL_INCLUDE_DIR NUMACTL_LIBRARY )
