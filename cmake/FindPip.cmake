######################################################
# - Try to find pip command
# Once done this will define
#  PIP_FOUND - System has pip
#  PIP_BINARY - Path to the pip binary file

######################################################
find_program(PIP_BINARY pip3)

######################################################
include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set PIP_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Pip  "Cannot find 'pip'. This is required beause you took the git master branch. Download official tar.gz or disable webview."
	PIP_BINARY)

######################################################
mark_as_advanced(PIP_BINARY)

