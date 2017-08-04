#!/bin/bash
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
NUMAPROF_PREFIX=
PINTOOL_PREFIX=

######################################################
CALLGRIND=0
case "$1" in
	--callgrind)
		CALLGRIND=1
		shift
		;;
	--help)
		echo "USAGE : $0 [--callgrind] {PRGM} [PGM OPTIONS]...."
		exit 0
		;;
esac

######################################################
${PINTOOL_PREFIX}/pin -t ${NUMAPROF_PREFIX}/lib/libnumaprof-pintool.so -- "$@" &
pid=$!
wait $pid
status=$?

######################################################
#convert to callgrind format
if [ "$CALLGRIND" = "1" ]
then
	echo "Converting to callgrind...."
	${NUMAPROF_PREFIX}/bin/numaprof-to-callgrind numaprof-$pid.json > numaprof-$pid.callgrind
	rm numaprof-$pid.json
fi

######################################################
exit $status
