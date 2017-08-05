# -*- coding: utf-8 -*-
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
from flask import Flask, render_template, send_from_directory
from ProfileHandler import ProfileHandler
import os

######################################################
app = Flask(__name__, static_url_path='')

######################################################
profile = ProfileHandler(os.environ["NUMAPROF_FILE"])

######################################################
@app.route('/')
@app.route('/index.html')
def root():
    return render_template('index.html', file=profile.getFileName(), page = "home")

@app.route('/static/<path:path>')
def serveStaticFiles(path):
    return send_from_directory('./static', path)

@app.route('/static/jquery/<path:path>')
def jqueryFiles(path):
    return send_from_directory('./bower_components/jquery/dist/', path)

@app.route('/static/bootstrap/<path:path>')
def bootsrapFiles(path):
    return send_from_directory('./bower_components/bootstrap/dist/', path)
