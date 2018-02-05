######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC OFF)

######################################################

#includes
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5WebEngineWidgets_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})

# path to prefix
add_definitions(-DNUMAPROF_PREFIX="${CMAKE_INSTALL_PREFIX}")

# Tell CMake to create the helloworld executable
add_executable(numaprof-qt5 main.cpp)

# Use the Widgets module from Qt 5.
target_link_libraries(numaprof-qt5 Qt5::Widgets Qt5::WebEngineWidgets Qt5::Network)

#install
install(TARGETS numaprof-qt5 DESTINATION bin)
