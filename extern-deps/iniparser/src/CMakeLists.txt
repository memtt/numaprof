######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.6                        #
#            DATE     : 06/2025                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
add_library(iniparser OBJECT dictionary.c iniparser.c)
SET_TARGET_PROPERTIES(iniparser PROPERTIES COMPILE_FLAGS -fPIC)
