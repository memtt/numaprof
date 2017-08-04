#!/bin/bash
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
NUMAPROF_PREFIX=
PINTOOL_PREFIX=

######################################################
${PINTOOL_PREFIX}/pin -t ${NUMAPROF_PREFIX}/lib/libnumaprof-pintool.so -- "$@"
exit $?
