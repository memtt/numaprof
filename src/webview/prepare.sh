#!/bin/bash
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
export PATH=./node_modules/.bin/:$PATH
SOURCE_DIR=$PWD

######################################################
if [ ! -z "$1" ]
then
	cp bower.json "$1"
	cd "$1"
fi

######################################################
if ! which pip3
then
	echo "Python 'pip3' command required to fetch the webview server dependencies, please install before proceeding or download a release archive instead of using master branch"
	exit 1
fi

######################################################
if ! which npm
then
	echo "Nodejs 'npm' command required to fetch the webview client dependencies, please install 'nodejs' before proceeding or download a release archive instead of using master branch"
	exit 1
fi

######################################################
#to avoid npm warnings
if [ ! -d 'package.json' ]
then
cat > 'package.json' << EOF
{
	"name": "numaprof-webview"
}
EOF
fi

######################################################
#There is an issue on recent ubuntu with pip --target
#One workaround is to also use --system on the line
#This work only on recent enougth version so we check
PIP_EXTRA_SYSTEM=""
if [ ! -z "$(pip3 install --help 2>&1 | grep -e --system)" ]
then
	PIP_EXTRA_SYSTEM="--system"
fi

######################################################
#Workaround on debian:jessie and ubuntu:16.X which rename
#node executable as nodejs which break bower
if ! which node
then
	if which nodejs
	then
		echo "Create NodeJS -> Node fix for ubuntu/debian !"
		mkdir -p ./node_modules/.bin/
		ln -s $(which nodejs) ./node_modules/.bin/node
	else
		echo "You should install NodeJS to fetch web GUI components"
		echo "Or download a release archive as it already contains all those files"
		exit 1
	fi
fi

######################################################
set -e
set -x
rm -rfd deps
pip3 install -t deps ${PIP_EXTRA_SYSTEM} --no-compile flask flask_httpauth htpasswd flask-htpasswd
npm install bower
bower install
rm -f ./node_modules/.bin/node
set +x
if [ ! -z "$1" ]; then
	echo > deps-loaded
fi

