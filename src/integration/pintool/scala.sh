#!/bin/bash
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
if [ -z "$1" ]; then
	echo "Usage : $0 {THREADS}"
	exit 1
fi

######################################################
THREADS=$1

######################################################
rm orig.txt instr.txt

######################################################
for threads in 1 2 4 8 16 `seq 32 32 256` `seq 320 64 512`
do 
	if [ $threads -le $THREADS ]
	then
		echo $threads
		/usr/bin/time -f "$threads\t%e" -o orig.txt --append ./a.out $threads
		/usr/bin/time -f "$threads\t%e" -o instr.txt --append ~/pin-3.2-81205-gcc-linux/pin -t obj-intel64/numaprof.so -- ./a.out $threads
	fi
done
