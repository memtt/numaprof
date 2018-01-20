######################################################
# - Try to find pintool (https://github.com/open-source-parsers/pintool/)
# Once done this will define
#  PINTOOL_FOUND - System has pintool
#  PINTOOL_PIN - Path to the pintool pin binary file

######################################################
set(PINTOOL_PREFIX "" CACHE STRING "Help cmake to find pintool library (https://software.intel.com/en-us/articles/pin-a-dynamic-binary-instrumentation-tool) into your system.")

######################################################
find_program(PINTOOL_PIN pin
	HINTS ${PINTOOL_PREFIX}/)

######################################################
include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set PINTOOL_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Pintool  DEFAULT_MSG
	PINTOOL_PIN)

######################################################
mark_as_advanced(PINTOOL_PIN)

