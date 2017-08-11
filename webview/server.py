# -*- coding: utf-8 -*-
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
from flask import Flask, render_template, send_from_directory, Response, send_file, abort
from ProfileHandler import ProfileHandler
import os
import json
from flask_httpauth import HTTPBasicAuth
from nocache import nocache
from flask_cache import Cache
import argparse
import sys

######################################################
app = Flask(__name__, static_url_path='')
#app.config["CACHE_TYPE"] = "null"
#cache = Cache(app,config={'CACHE_TYPE': 'null'})
#cache.init_app(app)

######################################################
auth = HTTPBasicAuth()
users = {
    "admin": "admin"
}

@auth.get_password
def get_pw(username):
	if username in users:
		return users.get(username)
	return None

######################################################
parser = argparse.ArgumentParser(description='Numaprof web server.')
parser.add_argument('profileFile', metavar='FILE', type=str,
                    help='Numaprof profile files to display')
parser.add_argument('--override', '-O', dest='override',
                    help='Override file path, format: "/orig/:/new/,/orig2/:/new2/"')

args = parser.parse_args()

######################################################
gblPathReplace = {};
if args.override != None:
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
print " * Loading file..."
profile = ProfileHandler(args.profileFile)
print " * Done"

######################################################
@app.route('/')
@app.route('/index.html')
@auth.login_required
@nocache
def rootPage():
    return render_template('index.html', file=profile.getFileName(), page = "home")

@app.route('/threads.html')
@auth.login_required
@nocache
def threadPage():
    return render_template('threads.html', file=profile.getFileName(), page = "threads")

@app.route('/sources.html')
@auth.login_required
@nocache
def sourcesPage():
    return render_template('sources.html', file=profile.getFileName(), page = "sources")

@app.route('/details.html')
@auth.login_required
@nocache
def detailsPage():
    return render_template('details.html', file=profile.getFileName(), page = "details")

@app.route('/help.html')
@auth.login_required
@nocache
def helpPage():
    return render_template('help.html', file=profile.getFileName(), page = "help")

######################################################
@app.route('/static/<path:path>')
@auth.login_required
@nocache
def serveStaticFiles(path):
    return send_from_directory('./static', path)

@app.route('/static/bower/<path:path>')
@auth.login_required
@nocache
def jqueryFiles(path):
    return send_from_directory('./bower_components/', path)

######################################################
@app.route('/api/index/infos.json')
@auth.login_required
@nocache
def apiIndexSummary():
	data = profile.getInfos()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

@app.route('/api/index/numa-topo.json')
@auth.login_required
@nocache
def apiIndexNumaTopo():
	data = profile.getNumaTopo()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

@app.route('/api/index/process-summary.json')
@auth.login_required
@nocache
def apiIndexProcessSummary():
	data = profile.getProcessSummary()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

@app.route('/api/index/process-access-matrix.json')
@auth.login_required
@nocache
def apiIndexProcessAccessMatrix():
	data = profile.getProcessAccessMatrix()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

@app.route('/api/index/numa-page-stats.json')
@auth.login_required
@nocache
def apiIndexNumaPageStats():
	data = profile.getNumaPageStats()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

######################################################
@app.route('/api/threads/infos.json')
@auth.login_required
@nocache
def apiThreadsInfos():
	data = profile.getThreadInfos()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

######################################################
@app.route('/api/sources/functions.json')
@auth.login_required
@nocache
def apiSourcesFuncions():
	data = profile.getFuncList()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

######################################################
@app.route('/api/details/threads.json')
@auth.login_required
@nocache
def apiDetailsThreads():
	data = profile.getThreadInfos()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

######################################################
@app.route('/api/sources/file-stats/<path:path>')
@auth.login_required
@nocache
def apiSourcesFileStats(path):
	path = "/"+path
	data = profile.getFileStats(path)
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

@app.route('/api/sources/file/<path:path>')
@auth.login_required
@nocache
def sourceFiles(path):
	path = "/"+path
	if profile.hasFile(path):
		path = replaceInPath(path)
		print path
		data=open(path).read()
		return data
	else:
		abort(404)

app.run(host="127.0.0.1", port=8080, threaded=True)
