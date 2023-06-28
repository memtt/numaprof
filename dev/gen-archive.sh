#!/bin/bash
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#This script aimed at generating archive to distribute
#the project. Simply call it from root directory without
#any agruments.

######################################################
#extract version
version=1.1.5
prefix=numaprof-${version}

######################################################
if [ ! -z "$(echo $version | grep dev)" ]; then
	prefix=${prefix}-$(git rev-parse --short HEAD)
fi

######################################################
echo "Generate ${prefix}.tar.gz..."
set -e
git archive --format=tar --prefix=${prefix}/ HEAD | bzip2 > /tmp/${prefix}.tar.bz2
current=$PWD
cd /tmp
tar -xf ${prefix}.tar.bz2
cd /tmp/${prefix}/src/webview
./prepare.sh
rm -rfd node_modules
cd /tmp
tar -cjf ${prefix}.tar.bz2 ${prefix}
cd $current
mv /tmp/${prefix}.tar.bz2 ./
rm -rfvd /tmp/${prefix}
echo "Finished"
