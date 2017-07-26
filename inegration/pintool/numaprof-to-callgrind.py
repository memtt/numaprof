#!/usr/bin/python
# -*- coding: utf-8 -*-
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
import json
import sys

######################################################
class Converter:
	def __init__(self,file):
		self.file = file		
		with open(file) as data_file:    
			self.data = json.load(data_file)

######################################################
if __name__ == "__main__":
	#check arg
	if len(sys.argv) != 2:
		print >>sys.stderr, "Require filename as parameter !"
		sys.exit(1)
	#laod file
	converter = Converter(sys.argv[1])
