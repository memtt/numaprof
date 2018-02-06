#!/bin/bash
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.0.0                        #
#            DATE     : 02/2018                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#export FLASK_APP=server.py
export PYTHONPATH=$PWD/deps/:$PYTHONPATH
python server.py "$@"
exit $?
