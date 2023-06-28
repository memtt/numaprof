#!/usr/bin/env python3
# -*- coding: utf-8 -*-
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
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
			"mcdramAccess",
			"nonAlloc"
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

	def getZeros(self):
		lst = []
		for counter in self.counters:
			lst.append(0)
		return " ".join(str(x) for x in lst)

	def buildHeader(self):
		print("version: 1")
		print("creator: numaprof")
		print("pid: 10719")
		print("cmd: ./a.out")
		print("part: 1")
		print("")
		print("position: line")
		#instr
		print("event: firstTouch : First touch")
		print("event: unpinnedFirstTouch : Unpinned first touch")
		print("event: unpinnedPageAccess : Unpinned page access")
		print("event: unpinnedThreadAccess : Unpinned thread access")
		print("event: unpinnedBothAccess : Unpinned both access")
		print("event: localAccess : Local NUMA access")
		print("event: remoteAccess : Remote NUMA access")
		print("event: mcdramAccess : KNL MCDRAM access")
		print("event: nonAlloc : Access to static allocated elements")
		#allocs
		print("event: allocfirstTouch : Allocation first touch")
		print("event: allocunpinnedFirstTouch : Allocation unpinned first touch")
		print("event: allocunpinnedPageAccess : Allocation unpinned page access")
		print("event: allocunpinnedThreadAccess : Allocation unpinned thread access")
		print("event: allocunpinnedBothAccess : Allocation unpinned both access")
		print("event: alloclocalAccess : Allocation local NUMA access")
		print("event: allocremoteAccess : Allocation remote NUMA access")
		print("event: allocmcdramAccess : Allocation KNL MCDRAM access")
		print("event: allocnonAlloc : Allocation access to static allocated elements")
		print("events:"," ".join(self.counters)," ".join("alloc"+x for x in self.counters))
		print("summary:",self.computeSummary())
		print("")

	def getObjString(self,strings,obj,key):
		if key in obj:
			return strings[obj[key]]
		else:
			return "??"

	def getLocation(self,instr):
		strings = self.data["symbols"]["strings"]
		instructions = self.data["symbols"]["instr"]
		tmp = instructions[instr]
		ret = {
			"file": self.getObjString(strings,tmp,"file"),
			"function": self.getObjString(strings,tmp,"function"),
			"binary": self.getObjString(strings,tmp,"binary"),
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

	def buildContentStack(self):
		for entry in self.data["instructions"]:
			for instr in entry["stack"]:
				location = self.getLocation(instr)
				print("ob=%s"%(location["binary"]))
				print("fl=%s"%(location["file"]))
				print("fn=%s"%(location["function"]))
			print("%d %s %s"%(location["line"],self.statToString(entry["stats"]),self.getZeros()))
			print("")
		for entry in self.data["allocs"]:
			for instr in entry["stack"]:
				location = self.getLocation(instr)
				print("ob=%s"%(location["binary"]))
				print("fl=%s"%(location["file"]))
				print("fn=%s"%(location["function"]))
			print("%d %s %s"%(location["line"],self.getZeros(),self.statToString(entry["stats"])))
			print("")

	def buildContentNoStack(self):
		for instr in self.data["instructions"]:
			location = self.getLocation(instr)
			print("ob=%s"%(location["binary"]))
			print("fl=%s"%(location["file"]))
			print("fn=%s"%(location["function"]))
			print("%d %s %s"%(location["line"],self.statToString(self.data["instructions"][instr]),self.getZeros()))
			print("")
		for instr in self.data["allocs"]:
			location = self.getLocation(instr)
			print("ob=%s"%(location["binary"]))
			print("fl=%s"%(location["file"]))
			print("fn=%s"%(location["function"]))
			print("%d %s %s"%(location["line"],self.getZeros(),self.statToString(self.data["allocs"][instr])))
			print("")

	def buildContent(self):
		if type(self.data["instructions"]) is dict:
			self.buildContentNoStack();
		else:
			self.buildContentStack();

	def convert(self):
		self.buildHeader()
		self.buildContent()

######################################################
if __name__ == "__main__":
	#check arg
	if len(sys.argv) != 2:
		print("Require filename as parameter !", file=sys.stderr)
		sys.exit(1)
	#laod file
	converter = Converter(sys.argv[1])
	converter.convert()
