#!/bin/bash
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.6                        #
#            DATE     : 06/2025                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
if [ -z "$3" ]; then
	authFile=${HOME}/.numaprof/htpasswd
else
	authFile=$3
fi

######################################################
if [ ! -d ${HOME}/.numaprof ]; then
	mkdir ${HOME}/.numaprof
fi

######################################################
if [ ! -f "$authFile" ]; then
	touch "$authFile"
	chmod 600 "$authFile"
fi

######################################################
#export FLASK_APP=server.py
export PYTHONPATH=$PWD/deps/:$PYTHONPATH
python3 nhtpasswd.py "$@"
exit $?
