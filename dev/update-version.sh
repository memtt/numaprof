#!/bin/bash
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#Used to upate the version in all file headers and
#all project to be sync.
#At the end it list the files where the old version remains.
#To be fixed by hand.

######################################################
COLOR_RED="$(printf "\033[31m")"
COLOR_RESET="$(printf "\033[0m")"

######################################################
function extract_old_version()
{
	OLD_VERSION=$(cat configure | grep VERSION | xargs echo | cut -d " " -f 4)
	echo "Old version is ${OLD_VERSION}"
	OLD_VERSION_SAFE=$(echo $OLD_VERSION | sed -e "s/\\./\\\\./g")
	OLD_SHORT_VERSION=$(echo $OLD_VERSION | sed -e 's/-dev//g')
	OLD_SHORT_VERSION_SAFE=$(echo $OLD_SHORT_VERSION | sed -e "s/\\./\\\\./g")
}

######################################################
function print_error()
{
	echo "${COLOR_RED}$@${COLOR_RESET}" 1>&2
}

######################################################
#check args
if [ -z "$1" ]; then
	print_error "Missing version !"
	echo "Usage : $0 {VERSION}" 1>&2
	exit 1
fi

######################################################
#check call from root directory of project
if [ "$0" != "./dev/update-version.sh" ]; then
	print_error "Caution, you must call this script from the project root directory !"
fi

######################################################
version="$1"
newdate="DATE     : $(date +%m/%Y)"
V="VERSION"
newversion="$V  : $(printf "%-16s" "$version")"
versionStrict=$(echo $version | cut -f 1 -d -)

######################################################
extract_old_version

######################################################
find ./ | while read file
do
	#exclude directories
	if [ -d "$file" ]; then continue; fi

	#exclude some paths
	case "$file" in
		./.git/*|./build*)
			#echo "exclude $file"
			continue
			;;
	esac

	#exclude non nupaprof files
	if [ -z "$(cat $file | grep NUMAPROF)" ] && [ -z "$(cat $file | grep numaprof)" ]
	then
		continue;
	fi

	#do replacement
	sed -i -r -e "s#  DATE     : [0-9]{2}/[0-9]{4}#  ${newdate}#g" \
	          -e "s#  $V  : .{16}#  ${newversion}#" \
	          -e "s#  $V  : [0-9a-zA-Z.-]+\$#  $V  : ${version}#" \
			  "${file}"
done

######################################################
#manpages
for tmp in src/manpages/*.ronn
do
	sed -i -r -e "s/${OLD_SHORT_VERSION_SAFE}/${versionStrict}/g" $tmp
done
cur=$PWD
cd src/manpages/
make || exit 1
cd $cur

######################################################
#update bower and package
sed -i -r -e "s/^version=[0-9A-Za-z.-]+$/version=${version}/g" dev/gen-archive.sh
#sed -i -r -e "s/^PROJECT_NUMBER         = ${OLD_VERSION_SAFE}$/PROJECT_NUMBER         = ${version}/g" Doxyfile
sed -i -r -e "s/${OLD_VERSION_SAFE}/${versionStrict}/g" src/lib/CMakeLists.txt
#sed -i -r -e "s/${OLD_SHORT_VERSION_SAFE}/${versionStrict}/g" packaging/README.md
#sed -i -r -e "s/${OLD_SHORT_VERSION_SAFE}/${versionStrict}/g" dev/packaging.sh
#sed -i -r -e "s/${OLD_SHORT_VERSION_SAFE}/${versionStrict}/g" packaging/fedora/malt.spec
#sed -i -r -e "s/${OLD_SHORT_VERSION_SAFE}/${versionStrict}/g" packaging/debian/changelog
sed -i -r -e "s/${OLD_SHORT_VERSION_SAFE}/${versionStrict}/g" Doxyfile

######################################################
#serach not updated
echo "=================== Check not updated list ======================"
grep -RHn "$(echo "${OLD_VERSION}" | sed -e 's/-dev//g' -e 's/\./\\./g')" ./ | grep -v node_modules | grep -v extern-deps | grep -v "\.git" | grep -v bower_components | grep -v deps | grep -v build | grep "$(echo ${OLD_VERSION} | sed -e 's/-dev//g')"
echo "================================================================="
