#!/bin/bash
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.0-dev                    #
#            DATE     : 02/2018                      #
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
if ! which pip
then
	echo "Python 'pip' command required to fetch the webview server dependencies, please install before proceeding or download a release archive instead of using master branch"
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
if [ ! -z "$(pip install --help 2>&1 | grep -e --system)" ]
then
	PIP_EXTRA_SYSTEM="--system"
fi

######################################################
set -e
set -x
pip install -t deps ${PIP_EXTRA_SYSTEM} --no-compile flask flask_httpauth Flask-Cache htpasswd flask-htpasswd
npm install bower
bower install
set +x
if [ ! -z "$1" ]; then
	echo > deps-loaded
fi
