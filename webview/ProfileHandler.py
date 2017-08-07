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
