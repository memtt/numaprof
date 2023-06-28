######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
add_executable(main-simple main-simple.cpp)
SET_TARGET_PROPERTIES(main-simple PROPERTIES COMPILE_FLAGS "-g -O0")

######################################################
add_executable(main-threads main-threads.cpp)
SET_TARGET_PROPERTIES(main-threads PROPERTIES COMPILE_FLAGS "-g -pthread -O0")
target_link_libraries(main-threads "-pthread")

######################################################
add_executable(main-1G-big-alloc main-1G-big-alloc.cpp)
SET_TARGET_PROPERTIES(main-simple PROPERTIES COMPILE_FLAGS "-g -O0")

######################################################
add_executable(main-1G-small-alloc main-1G-small-alloc.cpp)
SET_TARGET_PROPERTIES(main-simple PROPERTIES COMPILE_FLAGS "-g -O0")

######################################################
find_package(OpenMP)

######################################################
add_executable(case-openmp-loop case-openmp-loop.cpp)
SET_TARGET_PROPERTIES(case-openmp-loop PROPERTIES COMPILE_FLAGS "-g -O0 ${OpenMP_CXX_FLAGS}")
SET_TARGET_PROPERTIES(case-openmp-loop PROPERTIES LINK_FLAGS "${OpenMP_CXX_FLAGS}")

######################################################
add_executable(case-omp-stack-issue case-omp-stack-issue.cpp)
SET_TARGET_PROPERTIES(case-omp-stack-issue PROPERTIES COMPILE_FLAGS "-g -O0 ${OpenMP_CXX_FLAGS}")
SET_TARGET_PROPERTIES(case-omp-stack-issue PROPERTIES LINK_FLAGS "${OpenMP_CXX_FLAGS}")


