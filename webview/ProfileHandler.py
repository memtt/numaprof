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

######################################################
class ProfileHandler:
	metrics = [ "firstTouch", "unpinnedFirstTouch", "unpinnedPageAccess", "unpinnedThreadAccess", "unpinnedBothAccess", "localAccess", "remoteAccess", "mcdramAccess" ]
	
	def __init__(self,filepath):
		self.filepath = filepath
		self.load()
	
	def load(self):
		json_data=open(self.filepath).read()
		self.data = json.loads(json_data)
	
	def getFileName(self):
		return self.filepath.split('/')[-1]

	def getInfos(self):
		return self.data["infos"]

	def getNumaTopo(self):
		return self.data["topo"]

	def getThreadInfos(self):
		return self.data["threads"]

	def getProcessSummary(self):
		#init
		total = {}
		for m in self.metrics:
			total[m] = 0
		
		#sum all threads
		for thread in self.data["threads"]:
			for m in thread["stats"]:
				total[m] += thread["stats"][m]
		
		#ret
		return {
			"metrics": total
		}
	
	def getNumaNodes(self):
		cnt = 0
		for topo in self.data["topo"]:
			cnt += 1
		return cnt
	
	def getNumaPageStats(self):
		return self.data["process"]["maxAllocatedPages"]

	def getProcessAccessMatrix(self):
		out = {}
		#init
		for node in range(-1,self.getNumaNodes()):
			out[str(node)] = []
			for i in range(0,self.getNumaNodes()):
				out[str(node)].append(0)

		#merge
		for thread in self.data["threads"]:
			matrix = thread["accessMatrix"]
			for node in range(-1,self.getNumaNodes()):
				for i in range(0,self.getNumaNodes()):
					out[str(node)][i] += matrix[str(node)][i]
		#ret
		return out
	
	def getFuncName(self,instr):
		id = self.data["symbols"]["instr"][instr]["function"]
		return self.data["symbols"]["strings"][id]
	
	def merge(self,out,inData):
		for entry in inData:
			if entry in out:
				out[entry] += inData[entry]
			else:
				out[entry] = inData[entry]
	
	def getDefault(self):
		out = {"access":{},"alloc":{}};
		for i in self.metrics:
			out["access"][i] = 0
			out["alloc"][i] = 0
		return out
	
	def getFuncList(self):
		out = {}
	
		#do it for instructions
		for instr in self.data["instructions"]:
			fname = self.getFuncName(instr)
			if not fname in out:
				out[fname] = self.getDefault()
			if not "access" in out[fname]:
				out[fname]["access"] = {}
			self.merge(out[fname]["access"],self.data["instructions"][instr])
		
		#do it for allocs
		for instr in self.data["allocs"]:
			fname = self.getFuncName(instr)
			if not fname in out:
				out[fname] = self.getDefault()
			if not "alloc" in out[fname]:
				out[fname]["alloc"] = {}
			self.merge(out[fname]["alloc"],self.data["allocs"][instr])
		return out;
