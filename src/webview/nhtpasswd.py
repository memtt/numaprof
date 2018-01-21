# -*- coding: utf-8 -*-
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 07/2017                      #
#            AUTHOR   : Valat Sébastien  - CERN      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
#import
import sys
import htpasswd
import getpass
from os.path import expanduser

######################################################
#help
if len(sys.argv) == 1:
	print "Missing argument, usage : numaprof-htpasswd {USER} [PASSWORD]"
	sys.exit(1)

######################################################
#extract infos
user = sys.argv[1]

#pass
if len(sys.argv) == 2:
	password = getpass.getpass()
elif len(sys.argv) == 3:
	password = sys.argv[2]
else:
	print "Too many arguments !"
	sys.exit(1)

######################################################
#add or set in db
with htpasswd.Basic(expanduser("~")+"/.numaprof/htpasswd") as userdb:
	try:
		userdb.add(user, password)
	except htpasswd.basic.UserExists, e:
		userdb.change_password(user,password)
