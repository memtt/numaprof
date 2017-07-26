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
		self.counters = [
			"firstTouch",
			"unpinnedFirstTouch",
			"unpinnedPageAccess",
			"unpinnedThreadAccess",
			"unpinnedBothAccess",
			"localAccess",
			"remoteAccess",
			"mcdramAccess"
		]
		with open(file) as data_file:    
			self.data = json.load(data_file)
	
	def computeSummary(self):
		threads = self.data["threads"];
		summary = {}
		for counter in self.counters:
			summary[counter] = 0
		for thread in threads:
			for entry in thread["stats"]:
				summary[entry] += thread["stats"][entry]
		lst = []
		for counter in self.counters:
			lst.append(summary[counter])
		return " ".join(str(x) for x in lst)

	def statToString(self,stats):
		lst = []
		for counter in self.counters:
			if counter in stats:
				lst.append(stats[counter])
			else:
				lst.append(0)
		return " ".join(str(x) for x in lst)

	def buildHeader(self):
		print "version: 1"
		print "creator: numaprof"
		print "pid: 10719"
		print "cmd: ./a.out"
		print "part: 1"
		print ""
		print "position: line"
		print "events:"," ".join(self.counters)
		print "summary:",self.computeSummary()
		print ""

	def getLocation(self,instr):
		strings = self.data["symbols"]["strings"]
		instructions = self.data["symbols"]["instr"]
		tmp = instructions[instr]
		ret = {
			"file": strings[tmp["file"]],
			"function": strings[tmp["function"]],
			"binary": strings[tmp["binary"]],
		}
		if ret["file"] == "??":
			ret["file"] = "unknwon"
		if ret["function"] == "??":
			ret["function"] = "unknwon"
		if ret["binary"] == "??":
			ret["binary"] = "unknwon"
		if "line" in tmp:
			ret["line"] = tmp["line"]
		else:
			ret["line"] = 0
		return ret

	def buildContent(self):
		for instr in self.data["instructions"]:
			location = self.getLocation(instr)
			print "ob=%s"%(location["binary"])
			print "fl=%s"%(location["file"])
			print "fn=%s"%(location["function"])
			print "%d %s"%(location["line"],self.statToString(self.data["instructions"][instr]))
			print ""

	def convert(self):
		self.buildHeader()
		self.buildContent()

######################################################
if __name__ == "__main__":
	#check arg
	if len(sys.argv) != 2:
		print >>sys.stderr, "Require filename as parameter !"
		sys.exit(1)
	#laod file
	converter = Converter(sys.argv[1])
	converter.convert()