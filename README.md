Numaprof
========

[![Build Status](https://travis-ci.org/memtt/numaprof.svg?branch=master)](https://travis-ci.org/memtt/numaprof)

What is it ?
------------

Numaprof is a NUMA memory profiler. The idea is to instrument the read/write operations in 
the application and check the NUMA location of the thread at 
accesss time to compare it to the memory location of the data.

The tool is currently based on Pintool, a dynamic instrumentation tool from Intel offering a little bit
the same service than valgrind but supporting threads so faster for parallel applications.

You can find more details and screenshots on the dedicated website: https://memtt.github.io/numaprof/.

![NUMAPROF GUI](https://memtt.github.io/numaprof/images/screenshots/screenshot-6-2.png)

Metrics
-------

Numaprof extract the given metrics per call site and per malloc call site :

 * firstTouch : permet de savoir où ont lieu les first touch depuis un thread bindé
 * unpinnedFirstTouch : permet de savoir où ont lieu les first touch depuis des thread non bindés
 * localAccess : Permet de compter les accès locaux (via un thread bindé)
 * remoteAccess : Permet de compter les accès distant (via un thread bindé)
 * unpinnedPageAccess : Accès depuis un thread bindé à une page dont le thread ayant fait le first touch était non bindé
 * unpinnedThreadAccess : Accès depuis un thread non bindé à une page dont le thread ayant fait le first touch était bindé
 * unpinnedBothAccess : Accès depuis un thread non bindé à une page mise en place par un thread non bindé
 * mcdram : Accès à la mcdram sur KNL
 
Dependencies
------------

NUMAPROF needs:

 * CMake (required to build, greated than 2.8.8) : https://cmake.org/
 * Intel Pintool (required, tested : 3.5 - 3.7) : https://software.intel.com/en-us/articles/pin-a-binary-instrumentation-tool-downloads. Take care of the licence which is free only for non commercial use.
 * Python (required). To run the webserver.
 * Qt5-webkit (optional, greater than : 5.4). To provide a browser embedded view to use ssh X forward instead of the webserver port forwarding.
 * libnuma or numactl devel package. This is required to use the profiler.

If you use the git repo (eg. master branch) instead of a release file :

 * NodeJS/npm. To fetch the JavaScript libraries used by the web GUI. If you use a release archive, they already contain all the required JS files so you don't need anymore NodeJS.
 * Python pip. To download the dependencies of the web server for the web GUI. Again you can use a release archive which already contain all those files.

If you don't have npm and pip on your server, prefer using the release archive which already contain all the required
libraries and do not depend anymore on those two commands.

Install
-------

First download the last version of pintool (tested : 3.5 on x86_64 arch : https://software.intel.com/en-us/articles/pin-a-binary-instrumentation-tool-downloads) and extract it somewhere.
TAKE CARE, PINTOOL IS NOT OPEN-SOURCE AND IS FREE ONLY FOR NON-COMMERCIAL USE.

Then use the configure script :

```
mkdir build
cd build
../configure --prefix=PREFIX --with-pintool=PINTOOL_PATH
make
make install
```

Usage
-----

Setup your paths (you can also use absolute paths if you don't want to change your env): 

```
export PATH=PREFIX/bin:$PATH
```

Run you program using the wrapper:

```
numaprof ./benchmark --my-option
```

The numaprof GUI is based on a webserver and be viewed in the browser at http://localhost:8080.
The GUI password is currently fixed to admin/admin. You can launch the webserver by running : 

```
numaprof-webview numaprof-1234.json
```

The first time you launch the GUI you will need to provide a user/password to secure the interface.
You can change the password or add other users by using :

```
numaprof-passwd {USER}
```

The users are stored into `~/.numaprof/htpasswd` by following the `htpasswd` format.

If you run the webview on a remote node, you can forward the http session to your local browser by using :

```
ssh myhost -L8080:localhost:8080
```

If you have Qt5-webkit installed you can also automatically open a bowser view by using ssh X-Forward by using :

```
numaprof-qt5 numaprof-1234.json
```

Kcachgrind compatibility
------------------------

If you want to generate the callgrind compatible output, use:

```
numaprof-to-callgrind numaprof-45689.json
```

Then you can open the callgrind file with kcachegrind (http://kcachegrind.sourceforge.net/html/Home.html):

```
kcachegrind numaprof-12345.callgrind
```

Available options
-----------------

Here the config file which can be given by using `-c FILE` option to numaprof-pintool. You can also give a specific entry
by using `-o SECTION:NAME=value,SECTION2:NAME2=value2`.

```ini
[output]
name=numaprof-%1-%2.%3
indent=true
json=true
dumpConfig=false
silent=false
removeSmall=false
removeRatio=0.5

[core]
skipStackAccesses=true
threadCacheEntries=512
objectCodePinned=false
skipBinaries=
accessBatchSize=0

[info]
hidden=false

[cache]
;can be 'dummy' or 'L1'
type=dummy
size=32768
associativity=8
```

On huge application
-------------------

NUMAPROF was not yet testing on multi-million line application so we expect some slow down on such big code.
But it should be able to work. Although, the web GUI might lag due to too much data. In this case, enable
filtering option at profiling time by using option to remove all entries smaller than 0.2% from the output profile:

```
numaprof-pintool -o output:removeSmall=true,output:removeRatio=0.2 ./benchmark --my-option
```

View on another machine
-----------------------

If you want to view the NUMAPROF profile on another machine than the one you profiled on, you can
copy the json file and open it. Ideally the sources need to be placed at the same path than the one
where you profiled.

If this is not the case you can use the override option of the GUI to redirect some directories :

```sh
numaprof-webview -o /home/my_server_user/server_path/project:/home/my_local_user/loal_path/project ./numaprof-1234.json
```

numactl
-------

If you want to profile an application while using the `numactl` tool to setup the memory binding you need to use
the command line in given order:

```sh
numactl {OPTIONS} numaprof-pintool ./MY_APP
```

Pointers:
---------

If you search pointers about similar tools, interesting related papers, you can refer to the [docs/bibliography.md](https://github.com/memtt/numaprof/blob/master/doc/bibliography.md) file.

License
-------

Numaprof is distributed under CeCILL-C licence which is LGPL compatible.
Take care, NUMAPROF currently strongly depend on Intel Pintool which is free only for non commercial use.

I would like to make a port to DynamoRIO to avoid this, if someone want to help !.

Discussion
----------

You can join the google group to exchange ideas and ask questions : https://groups.google.com/forum/#!forum/memtt-numaprof.