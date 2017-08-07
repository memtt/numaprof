# -*- coding: utf-8 -*-
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
from flask import Flask, render_template, send_from_directory, Response
from ProfileHandler import ProfileHandler
import os
import json
from flask_httpauth import HTTPBasicAuth
from nocache import nocache
from flask.ext.cache import Cache

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
profile = ProfileHandler(os.environ["NUMAPROF_FILE"])

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

@app.route('/help.html')
@auth.login_required
@nocache
def helpPage():
    return render_template('help.html', file=profile.getFileName(), page = "help")

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

@app.route('/api/threads/infos.json')
@auth.login_required
@nocache
def apiThreadsInfos():
	data = profile.getThreadInfos()
	jsonData = json.dumps(data)
	return Response(jsonData, mimetype='application/json')

app.run(host="127.0.0.1", port=8080, threaded=False)
