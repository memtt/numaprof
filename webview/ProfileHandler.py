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