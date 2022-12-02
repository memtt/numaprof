######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.4                        #
#            DATE     : 12/2022                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
add_library(iniparser OBJECT dictionary.c iniparser.c)
SET_TARGET_PROPERTIES(iniparser PROPERTIES COMPILE_FLAGS -fPIC)
