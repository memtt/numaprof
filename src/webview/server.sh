#!/bin/bash
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.7                        #
#            DATE     : 06/2026                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#export FLASK_APP=server.py
export PYTHONPATH=$PWD/deps/:$PYTHONPATH
python3 server.py "$@"
exit $?
