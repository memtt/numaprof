# -*- coding: utf-8 -*-
######################################################
#            PROJECT  : numaprof                     #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
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
	print("Missing argument, usage : numaprof-htpasswd {USER} [PASSWORD] [FILE]")
	sys.exit(1)

######################################################
#extract infos
user = sys.argv[1]

#file
authFile = expanduser("~")+"/.numaprof/htpasswd"

#pass
if len(sys.argv) == 2:
	password = getpass.getpass()
elif len(sys.argv) == 3:
	password = sys.argv[2]
elif len(sys.argv) == 4:
	password = sys.argv[2]
	authFile = sys.argv[3]
else:
	print("Too many arguments !")
	sys.exit(1)

######################################################
#add or set in db
with htpasswd.Basic(authFile) as userdb:
	try:
		userdb.add(user, password)
	except htpasswd.basic.UserExists as e:
		userdb.change_password(user,password)
