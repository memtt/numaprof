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

######################################################
app = Flask(__name__, static_url_path='')
app.config["CACHE_TYPE"] = "null"

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
def root():
    return render_template('index.html', file=profile.getFileName(), page = "home")

@app.route('/static/<path:path>')
@auth.login_required
@nocache
def serveStaticFiles(path):
    return send_from_directory('./static', path)

@app.route('/static/jquery/<path:path>')
@auth.login_required
@nocache
def jqueryFiles(path):
    return send_from_directory('./bower_components/jquery/dist/', path)

@app.route('/static/bootstrap/<path:path>')
@auth.login_required
@nocache
def bootsrapFiles(path):
    return send_from_directory('./bower_components/bootstrap/dist/', path)

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
