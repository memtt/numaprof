#!/bin/bash
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
export PATH=./node_modules/.bin/:$PATH

######################################################
set -e
set -x
pip install -t deps flask flask_httpauth
npm install bower
bower install
