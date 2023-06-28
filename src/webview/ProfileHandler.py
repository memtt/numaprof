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
import subprocess
import os

######################################################
class ProfileHandler:
	metrics = [ "firstTouch", "unpinnedFirstTouch", "unpinnedPageAccess", "unpinnedThreadAccess", "unpinnedBothAccess", "localAccess", "remoteAccess", "localMcdramAccess", "remoteMcdramAccess", "nonAlloc" ]
	
	def __init__(self,filepath):
		self.filepath = filepath
		self.load()
	
	def load(self):
		json_data=open(self.filepath).read()
		self.data = json.loads(json_data)
		version = self.data["infos"]["formatVersion"]
		if version != 2:
			raise Exception("Invalid format version, as {version} expected 2 !")
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

	def getProcessDistanceCnt(self):
		out = []
		#init
		for i in self.data["threads"][0]["distanceCnt"]:
			out.append(0)
		
		#merge
		for thread in self.data["threads"]:
			idx = 0
			for val in thread["distanceCnt"]:
				out[idx] += val
				idx += 1
		
		return out;
	
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
		#print self.fileFilter
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
			if entry == "line" or entry == "func" or entry == "file" or entry == "binary":
				continue;
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
		instructions = self.data["instructions"]
		for instr in instructions:
			#fname = self.getFuncName(instr)
			selectedInstructions = instructions[instr]
			fname = selectedInstructions["func"]
			if not fname in out:
				out[fname] = self.getDefault()
				#out[fname]["file"] = self.getFuncFileName(instr)
				out[fname]["file"] = selectedInstructions["file"]
			if not "access" in out[fname]:
				out[fname]["access"] = {}
			self.merge(out[fname]["access"],selectedInstructions)
		
		#do it for allocs
		allocs = self.data["allocs"]
		for instr in allocs:
			#fname = self.getFuncName(instr)
			allocInstr = allocs[instr]
			fname = allocInstr["func"]
			if not fname in out:
				out[fname] = self.getDefault()
				out[fname]["file"] = allocInstr["file"]
			if not "alloc" in out[fname]:
				out[fname]["alloc"] = {}
			self.merge(out[fname]["alloc"],allocInstr)
		return out;
	
	def getAsmFuncList(self):
		out = {}
	
		#do it for instructions
		instructions = self.data["instructions"]
		for instr in instructions:
			#fname = self.getFuncName(instr)
			selectedInstr = instructions[instr]
			fname = selectedInstr["func"]
			if not fname in out:
				out[fname] = self.getDefault()
				#out[fname]["file"] = self.getFuncFileName(instr)
				out[fname]["file"] = selectedInstr["binary"]
			if not "access" in out[fname]:
				out[fname]["access"] = {}
			self.merge(out[fname]["access"],selectedInstr)
		
		#do it for allocs
		allocs = self.data["allocs"]
		for instr in allocs:
			#fname = self.getFuncName(instr)
			allocInstr = allocs[instr]
			fname = allocInstr["func"]
			if not fname in out:
				out[fname] = self.getDefault()
				out[fname]["file"] = allocInstr["binary"]
			if not "alloc" in out[fname]:
				out[fname]["alloc"] = {}
			self.merge(out[fname]["alloc"],allocInstr)
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
	
	def getOneAddrOfSymbol(self,path,func):
		for instr in self.data["instructions"]:
			#fname = self.getFuncFileName(instr)
			fname = self.data["instructions"][instr]["binary"]
			f = self.data["instructions"][instr]["func"]
			if path == fname and f == func:
				return "0x"+self.getSoRelativeAddr(instr)
		
		for instr in self.data["allocs"]:
			#fname = self.getFuncFileName(instr)
			fname = self.data["allocs"][instr]["binary"]
			f = self.data["allocs"][instr]["func"]
			if path == fname and f == func:
				return "0x"+self.getSoRelativeAddr(instr)
	
	def loadBinaryDesass(self,path,func):
		addr = self.getOneAddrOfSymbol(path,func)[2:]
		data = subprocess.check_output(['objdump', '-d',path]).decode("utf-8")
		lines = data.split('\n')
		out = []
		status = False
		for line in lines:
			if status and line == "":
				out.append("")
				break
			if line == "":
				out = []
			else:
				caddr = line.split(":")[0].strip()
				if caddr == addr:
					status = True
				out.append(line)
		return "\n".join(out)

	def loadBinaryDesassMap(self,path,func):
		data = self.loadBinaryDesass(path,func)
		lines = data.split('\n')
		cnt = 1
		out = {}
		for line in lines:
			if len(line) > 0 and line[0] == ' ':
				addr = line.split(':')[0].strip()
				out[addr] = cnt
			cnt += 1
		return out
	
	def getSoRelativeAddr(self,addr):
		addrInt = int(addr[2:],16)
		for sym in self.data["symbols"]["map"]:
			lower = int(sym["lower"][2:],16)
			upper = int(sym["upper"][2:],16)
			if addrInt >= lower and addrInt < upper and (".so" in sym["file"] or lower != int('00400000',16)):
				baseAddr = int(self.data["symbols"]["lowAddrMap"][sym["file"]])
				#print "Replace in %s : %s => 0x%x"%(sym["file"],addr,(addrInt - lower))
				return "%x" % (addrInt - baseAddr)
		#print "Addr not found in memory map, keep addr : %s"%addr
		return addr[2:]
	
	def getAsmFileStats(self,path,realPath,func):
		out = {}
		asmMap = self.loadBinaryDesassMap(realPath,func)
		#do it for instructions
		for instr in self.data["instructions"]:
			#fname = self.getFuncFileName(instr)
			fname = self.data["instructions"][instr]["binary"]
			f = self.data["instructions"][instr]["func"]
			if fname == path and f == func:
				#line = self.getLine(instr)
				#addr = instr[2:]
				addr = self.getSoRelativeAddr(instr)
				#print addr, asmMap[addr]
				line = asmMap[addr]
				if not line in out:
					out[line] = self.getDefault()
					out[line]["func"] = f
				if not "access" in out[line]:
					out[line]["access"] = {}
				self.merge(out[line]["access"],self.data["instructions"][instr])
				out[line]["addr"] = addr
		
		#do it for allocs
		for instr in self.data["allocs"]:
			#fname = self.getFuncFileName(instr)
			fname = self.data["allocs"][instr]["binary"]
			f = self.data["allocs"][instr]["func"]
			if fname == path and f == func:
				#line = self.getLine(instr)
				#addr = instr[2:]
				addr = self.getSoRelativeAddr(instr)
				#print addr, asmMap[addr]
				line = asmMap[addr]
				if not line in out:
					out[line] = self.getDefault()
					out[line]["func"] = f
				if not "alloc" in out[line]:
					out[line]["alloc"] = {}
				self.merge(out[line]["alloc"],self.data["allocs"][instr])
				out[line]["addr"] = addr
		return out;
