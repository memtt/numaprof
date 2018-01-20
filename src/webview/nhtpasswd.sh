#!/bin/bash
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
if [ ! -d ${HOME}/.numaprof ]; then
	mkdir ${HOME}/.numaprof
fi

######################################################
if [ ! -f ${HOME}/.numaprof/htpasswd ]; then
	touch ${HOME}/.numaprof/htpasswd
fi

######################################################
#export FLASK_APP=server.py
export PYTHONPATH=$PWD/deps/:$PYTHONPATH
python nhtpasswd.py "$@"
exit $?
