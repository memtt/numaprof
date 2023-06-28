# -*- coding: utf-8 -*-
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
from flask import Flask, render_template, send_from_directory, Response, send_file, abort, request
from ProfileHandler import ProfileHandler
import os
import json
from flask_htpasswd import HtPasswdAuth
from nocache import nocache
import argparse
import sys
import subprocess
from os.path import expanduser
from random import randint
from random import choice
import string

######################################################
parser = argparse.ArgumentParser(description='Numaprof web server.')
parser.add_argument('profileFile', metavar='FILE', type=str,
                    help='Numaprof profile files to display')
parser.add_argument('--override', '-O', dest='override',
                    help='Override file path, format: "/orig/:/new/,/orig2/:/new2/"')
parser.add_argument('--search-path', '-S', dest='search',
                    help='Search file with non full path in this list of directory : "/home/orig/a,/tmp/b"')
parser.add_argument('--port', '-p', dest='port', type=int,
                    help='Port to use to export display to browser')
parser.add_argument('--webkit', '-w' ,dest='webkit', action='store_const',
                    const=1, default=0,
                    help='Automatically open a browser view using Qt webkit.')
parser.add_argument('--authfile', '-A', dest='authfile',
                    help='Provide the authentification file, used by QT GUI.')

args = parser.parse_args()

######################################################
app = Flask(__name__, static_url_path='')
#app.config["CACHE_TYPE"] = "null"
#cache = Cache(app,config={'CACHE_TYPE': 'null'})
#cache.init_app(app)

######################################################
webkitUser = ''.join(choice(string.ascii_uppercase + string.digits) for _ in range(32))
webkitPasswd = ''.join(choice(string.ascii_uppercase + string.digits) for _ in range(32))
#print webkitUser + " " + webkitPasswd

######################################################
authFile = expanduser("~")+'/.numaprof/htpasswd'
if args.webkit == 1:
	import htpasswd
	from tempfile import mkstemp
	tmp = mkstemp()
	authFile = tmp[1]
	os.fdopen(tmp[0],'w').close()
	with htpasswd.Basic(authFile) as userdb:
		try:
			userdb.add(webkitUser, webkitPasswd)
		except htpasswd.basic.UserExists as e:
			userdb.change_password(webkitUser,webkitPasswd)
elif args.authfile:
	authFile = args.authfile

######################################################
#config
app.config['FLASK_HTPASSWD_PATH'] = authFile
app.config['FLASK_SECRET'] = 'numaprof auth'

#build
htpasswd = HtPasswdAuth(app)

######################################################
if args.webkit == 1:
	#from https://mrl33h.de/post/23
	import platform
	import threading
	from PyQt5.QtCore import QUrl
	from PyQt5.QtWidgets import *
	from PyQt5.QtWebKitWidgets import QWebView
	
	#if int(platform.python_version_tuple()[1]) >= 6:
		#import asyncio
		#import asyncio.base_futures
		#import asyncio.base_tasks
		#import asyncio.compat
		#import asyncio.base_subprocess
		#import asyncio.proactor_events
		#import asyncio.constants
		#import asyncio.selector_events
		#import asyncio.windows_utils
		#import asyncio.windows_events

		#import jinja2.asyncsupport
		#import jinja2.ext

######################################################
port = 8080
if args.port is not None:
	port = args.port

######################################################
gblSearchPath = []
if args.search is not None:
	gblSearchPath = args.search.split(',')

######################################################
gblPathReplace = {};
if args.override is not None:
	lst = args.override.split(",")
	for over in lst:
		tmp = over.split(':')
		gblPathReplace[tmp[0]] = tmp[1]

######################################################
def replaceInPath(path):
	for repl in gblPathReplace:
		if repl in path:
			path = path.replace(repl,gblPathReplace[repl])
	return path

######################################################
def findSourceFile(name):
	for path in gblSearchPath:
		for root, dirs, files in os.walk(path):
			if name in files:
				return os.path.join(root, name)
	return False


######################################################
print (" * Loading file...")
profile = ProfileHandler(args.profileFile)

######################################################
#prepare some outputs
print (" * Prepare func list...")
functionsCache = json.dumps(profile.getFuncList())
print (" * Prepare asm func list...")
asmFunctionsCache = json.dumps(profile.getAsmFuncList())
print (" * Done")

######################################################
@app.route('/')
@app.route('/index.html')
@htpasswd.required
@nocache
def rootPage(user):
    return render_template('index.html', file=profile.getFileName(), page = "home")

@app.route('/threads.html')
@htpasswd.required
@nocache
def threadPage(user):
    return render_template('threads.html', file=profile.getFileName(), page = "threads")

@app.route('/sources.html')
@htpasswd.required
@nocache
def sourcesPage(user):
    return render_template('sources.html', file=profile.getFileName(), page = "sources")

@app.route('/details.html')
@htpasswd.required
@nocache
def detailsPage(user):
    return render_template('details.html', file=profile.getFileName(), page = "details")

@app.route('/asm.html')
@htpasswd.required
@nocache
def asmPage(user):
    return render_template('asm.html', file=profile.getFileName(), page = "asm")

@app.route('/help.html')
@htpasswd.required
@nocache
def helpPage(user):
    return render_template('help.html', file=profile.getFileName(), page = "help")

######################################################
@app.route('/static/<path:path>')
@htpasswd.required
@nocache
def serveStaticFiles(user,path):
    return send_from_directory('./static', path)

@app.route('/static/bower/<path:path>')
@htpasswd.required
@nocache
def jqueryFiles(user,path):
    return send_from_directory('./bower_components/', path)

######################################################
@app.route('/api/index/infos.json')
@htpasswd.required
@nocache
def apiIndexSummary(user):
	data = profile.getInfos()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

@app.route('/api/index/numa-topo.json')
@htpasswd.required
@nocache
def apiIndexNumaTopo(user):
	data = profile.getNumaTopo()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

@app.route('/api/index/process-summary.json')
@htpasswd.required
@nocache
def apiIndexProcessSummary(user):
	data = profile.getProcessSummary()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

@app.route('/api/index/process-access-matrix.json')
@htpasswd.required
@nocache
def apiIndexProcessAccessMatrix(user):
	data = profile.getProcessAccessMatrix()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

@app.route('/api/index/process-distance-cnt.json')
@htpasswd.required
@nocache
def apiIndexProcessDistanceCnt(user):
	data = profile.getProcessDistanceCnt()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

@app.route('/api/index/numa-page-stats.json')
@htpasswd.required
@nocache
def apiIndexNumaPageStats(user):
	data = profile.getNumaPageStats()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

######################################################
@app.route('/api/threads/infos.json')
@htpasswd.required
@nocache
def apiThreadsInfos(user):
	data = profile.getThreadInfos()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

######################################################
@app.route('/api/details/threads.json')
@htpasswd.required
@nocache
def apiDetailsThreads(user):
	data = profile.getThreadInfos()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

######################################################
@app.route('/api/sources/functions.json')
@htpasswd.required
@nocache
def apiSourcesFuncions(user):
	return Response(functionsCache, mimetype='application/json')

@app.route('/api/sources/file-stats/<path:path>')
@htpasswd.required
@nocache
def apiSourcesFileStats(user,path):
	path = fixPath(path)
	data = profile.getFileStats(path.replace('./',''))
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

def fixPath(path):
	if path.startswith('{.}'):
		return path.replace('{.}','.')
	elif path.startswith('/{.}/'):
		return path.replace('/{.}/','./')
	else:
		return "/"+path

@app.route('/api/sources/no-path-file-stats/<path:path>')
@htpasswd.required
@nocache
def apiSourcesNoPathFileStats(user,path):
	path = fixPath(path)
	data = profile.getFileStats(path.replace('./',''))
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

@app.route('/api/sources/file/<path:path>')
@htpasswd.required
@nocache
def sourceFiles(user,path):
	path = fixPath(path)
	if profile.hasFile(path.replace('./','')):
		path = replaceInPath(path)
		print (path)
		data=open(path).read()
		return data
	else:
		abort(404)

@app.route('/api/sources/no-path-file/<path:path>')
@htpasswd.required
@nocache
def sourceNoPathFiles(user,path):
	path = fixPath(path)
	fname = path.split('/')[-1]
	full = findSourceFile(fname)
	print (full)
	if full != False:
		data=open(full).read()
		return data
	else:
		abort(404)

######################################################
@app.route('/api/asm/functions.json')
@htpasswd.required
@nocache
def apiAsmFuncions(user):
	return Response(asmFunctionsCache, mimetype='application/json')

@app.route('/api/asm/file-stats/<path:path>')
@htpasswd.required
@nocache
def apiAsmFileStats(user,path):
	path = fixPath(path)
	func = request.args.get('func')
	data = profile.getAsmFileStats(path,replaceInPath(path),func)
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

@app.route('/api/asm/file/<path:path>')
@htpasswd.required
@nocache
def asmFiles(user,path):
	path = fixPath(path)
	func = request.args.get('func')
	print (func)
	if profile.hasBinary(path):
		path = replaceInPath(path)
		#data = subprocess.check_output(['objdump', '-d',path]).decode("utf-8") 
		data = profile.loadBinaryDesass(path,func)
		return data
	else:
		abort(404)

if args.webkit == 0:
	app.run(host="localhost", port=port, threaded=True)
else:
	#from https://mrl33h.de/post/23
	
	#select a port
	port = randint(1025, 65535)
	
	#start server
	thread = threading.Thread(target=app.run, args=['localhost', port])
	thread.daemon = True
	thread.start()

	qt_app = QApplication([])
	w = QWebView()
	url = QUrl('http://localhost:'+str(port))
	url.setUserName(webkitUser)
	url.setPassword(webkitPasswd)
	w.load(url)
	w.show()
	w.showMaximized()
	qt_app.exec_()
	os.remove(authFile)
