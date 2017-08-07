#!/bin/bash
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
if [ -z "$1" ]; then
	echo "You need to provide a file as parameter !"
	exit 1
fi

######################################################
export NUMAPROF_FILE="$1" 

######################################################
export FLASK_APP=server.py
export PYTHONPATH=$PWD/deps/:$PYTHONPATH
python -m flask run -p 8080 -h 127.0.0.1
exit $?
