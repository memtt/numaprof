######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.7                        #
#            DATE     : 06/2026                      #
#            AUTHOR   : Valat Sébastien              #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
add_library(numaprof-caches OBJECT CpuCache.cpp 
                                   CpuCacheDummy.cpp 
                                   CpuCacheBuilder.cpp
								   CpuSimpleFlatCache.cpp)
SET_TARGET_PROPERTIES(numaprof-caches PROPERTIES COMPILE_FLAGS -fPIC)
