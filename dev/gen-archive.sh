#!/bin/bash
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#This script aimed at generating archive to distribute
#the project. Simply call it from root directory without
#any agruments.

######################################################
#extract version
version=0.0.0-dev
prefix=numaprof-${version}

######################################################
echo "Generate ${prefix}.tar.gz..."
git archive --format=tar --prefix=${prefix}/ HEAD | gzip > ${prefix}.tar.gz
echo "Finished"
