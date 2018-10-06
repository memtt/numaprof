######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.0-dev                    #
#            DATE     : 02/2018                      #
#            AUTHOR   : Valat Sébastien              #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
add_library(numaprof-caches OBJECT CpuCache.cpp 
                                   CpuCacheDummy.cpp 
                                   CpuCacheBuilder.cpp)
SET_TARGET_PROPERTIES(numaprof-caches PROPERTIES COMPILE_FLAGS -fPIC)
