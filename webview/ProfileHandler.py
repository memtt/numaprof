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
import subprocess
import os

######################################################
class ProfileHandler:
	metrics = [ "firstTouch", "unpinnedFirstTouch", "unpinnedPageAccess", "unpinnedThreadAccess", "unpinnedBothAccess", "localAccess", "remoteAccess", "mcdramAccess", "nonAlloc" ]
	
	def __init__(self,filepath):
		self.filepath = filepath
		self.load()
	
	def load(self):
		json_data=open(self.filepath).read()
		self.data = json.loads(json_data)
		self.prepare()

	def prepare(self):
		self.prepareFileFilter();
		self.prepareFileNameAndLine();
	
	def prepareFileNameAndLine(self):
		#do it for instructions
		for instr in self.data["instructions"]:
			fname = self.getFuncFileName(instr)
			func = self.getFuncName(instr)
			line = self.getLine(instr)
			binary = self.getBinary(instr)
			self.data["instructions"][instr]["binary"] = binary
			self.data["instructions"][instr]["file"] = fname
			self.data["instructions"][instr]["func"] = func
			self.data["instructions"][instr]["line"] = line
		
		#do it for allocs
		for instr in self.data["allocs"]:
			fname = self.getFuncFileName(instr)
			func = self.getFuncName(instr)
			line = self.getLine(instr)
			binary = self.getBinary(instr)
			self.data["allocs"][instr]["binary"] = binary
			self.data["allocs"][instr]["file"] = fname
			self.data["allocs"][instr]["func"] = func
			self.data["allocs"][instr]["line"] = line
	
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
	
	def prepareFileFilter(self):
		out1 = {}
		out2 = {}
		for instr in self.data["symbols"]["instr"]:
			if instr in self.data["symbols"]["instr"] and "file" in self.data["symbols"]["instr"][instr]:
				fid = self.data["symbols"]["instr"][instr]["file"]
				fname = os.path.normpath(self.data["symbols"]["strings"][fid])
				out1[fname] = True
				
				bid = self.data["symbols"]["instr"][instr]["binary"]
				bname = os.path.normpath(self.data["symbols"]["strings"][bid])
				out2[bname] = True
		self.fileFilter = out1
		self.binaryFilter = out2
	
	def hasFile(self,fname):
		print self.fileFilter
		return fname in self.fileFilter
	
	def hasBinary(self,fname):
		return fname in self.binaryFilter
	
	def getFuncName(self,instr):
		id = self.data["symbols"]["instr"][instr]["function"]
		return self.data["symbols"]["strings"][id]
	
	def getFuncFileName(self,instr):
		if "file" in self.data["symbols"]["instr"][instr]:
			id = self.data["symbols"]["instr"][instr]["file"]
			return os.path.normpath(self.data["symbols"]["strings"][id])
		else:
			return "??"

	def getLine(self,instr):
		if "line" in self.data["symbols"]["instr"][instr]:
			return self.data["symbols"]["instr"][instr]["line"]
		else:
			return "??"
	
	def getBinary(self,instr):
		if "binary" in self.data["symbols"]["instr"][instr]:
			id = self.data["symbols"]["instr"][instr]["binary"]
			return os.path.normpath(self.data["symbols"]["strings"][id])
		else:
			return "??"

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
			#fname = self.getFuncName(instr)
			fname = self.data["instructions"][instr]["func"]
			if not fname in out:
				out[fname] = self.getDefault()
				#out[fname]["file"] = self.getFuncFileName(instr)
				out[fname]["file"] = self.data["instructions"][instr]["file"]
			if not "access" in out[fname]:
				out[fname]["access"] = {}
			self.merge(out[fname]["access"],self.data["instructions"][instr])
		
		#do it for allocs
		for instr in self.data["allocs"]:
			#fname = self.getFuncName(instr)
			fname = self.data["allocs"][instr]["func"]
			if not fname in out:
				out[fname] = self.getDefault()
				out[fname]["file"] = self.data["allocs"][instr]["file"]
			if not "alloc" in out[fname]:
				out[fname]["alloc"] = {}
			self.merge(out[fname]["alloc"],self.data["allocs"][instr])
		return out;
	
	def getAsmFuncList(self):
		out = {}
	
		#do it for instructions
		for instr in self.data["instructions"]:
			#fname = self.getFuncName(instr)
			fname = self.data["instructions"][instr]["func"]
			if not fname in out:
				out[fname] = self.getDefault()
				#out[fname]["file"] = self.getFuncFileName(instr)
				out[fname]["file"] = self.data["instructions"][instr]["binary"]
			if not "access" in out[fname]:
				out[fname]["access"] = {}
			self.merge(out[fname]["access"],self.data["instructions"][instr])
		
		#do it for allocs
		for instr in self.data["allocs"]:
			#fname = self.getFuncName(instr)
			fname = self.data["allocs"][instr]["func"]
			if not fname in out:
				out[fname] = self.getDefault()
				out[fname]["file"] = self.data["allocs"][instr]["binary"]
			if not "alloc" in out[fname]:
				out[fname]["alloc"] = {}
			self.merge(out[fname]["alloc"],self.data["allocs"][instr])
		return out;

	def getFileStats(self,path):
		out = {}
		#do it for instructions
		for instr in self.data["instructions"]:
			#fname = self.getFuncFileName(instr)
			fname = self.data["instructions"][instr]["file"]
			if fname == path:
				#line = self.getLine(instr)
				line = self.data["instructions"][instr]["line"]
				if not line in out:
					out[line] = self.getDefault()
					out[line]["func"] = self.data["instructions"][instr]["func"]
				if not "access" in out[line]:
					out[line]["access"] = {}
				self.merge(out[line]["access"],self.data["instructions"][instr])
		
		#do it for allocs
		for instr in self.data["allocs"]:
			#fname = self.getFuncFileName(instr)
			fname = self.data["allocs"][instr]["file"]
			if fname == path:
				#line = self.getLine(instr)
				line = self.data["allocs"][instr]["line"]
				if not line in out:
					out[line] = self.getDefault()
					out[line]["func"] = self.data["allocs"][instr]["func"]
				if not "alloc" in out[line]:
					out[line]["alloc"] = {}
				self.merge(out[line]["alloc"],self.data["allocs"][instr])
		return out;

	def loadBinaryDesassMap(self,path):
		data = subprocess.check_output(['objdump', '-d',path]).decode("utf-8")
		lines = data.split('\n')
		cnt = 1
		out = {}
		for line in lines:
			if len(line) > 0 and line[0] == ' ':
				addr = line.split(':')[0][2:]
				out[addr] = cnt
			cnt += 1
		return out
	
	def getAsmFileStats(self,path,realPath):
		out = {}
		asmMap = self.loadBinaryDesassMap(realPath)
		#do it for instructions
		for instr in self.data["instructions"]:
			#fname = self.getFuncFileName(instr)
			fname = self.data["instructions"][instr]["binary"]
			if fname == path:
				#line = self.getLine(instr)
				addr = instr[2:]
				#print addr, asmMap[addr]
				line = asmMap[addr]
				if not line in out:
					out[line] = self.getDefault()
					out[line]["func"] = self.data["instructions"][instr]["func"]
				if not "access" in out[line]:
					out[line]["access"] = {}
				self.merge(out[line]["access"],self.data["instructions"][instr])
				out[line]["addr"] = addr
		
		#do it for allocs
		for instr in self.data["allocs"]:
			#fname = self.getFuncFileName(instr)
			fname = self.data["allocs"][instr]["binary"]
			if fname == path:
				#line = self.getLine(instr)
				addr = instr[2:]
				#print addr, asmMap[addr]
				line = asmMap[addr]
				if not line in out:
					out[line] = self.getDefault()
					out[line]["func"] = self.data["allocs"][instr]["func"]
				if not "alloc" in out[line]:
					out[line]["alloc"] = {}
				self.merge(out[line]["alloc"],self.data["allocs"][instr])
				out[line]["addr"] = addr
		return out;
