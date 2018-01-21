Numaprof
========

What is it ?
------------

Numaprof is a NUMA memory profiler. The idea is to instrument the read/write operations in 
the application and check the NUMA location of the thread at 
accesss time to compare it to the memory location of the data.

The tool is currently based on Pintool, a dynamic instrumentation tool from Intel offering a little bit
the same service than valgrind but supporting threads so faster for parallel applications.

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

Install
-------

First download the last version of pintool (tested : 3.5 on x86_64 arch : https://software.intel.com/en-us/articles/pin-a-binary-instrumentation-tool-downloads) and extract it somewhere.

Then use the configure script (for pintool mode do not use directly the cmake script, the configure script does extra things with pin):

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

If you have PyQt5 installed you can also automatically open a bowser view by using :

```
numaprof-webview --webkit numaprof-1234.json
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

[info]
hidden=false
```

On huge application
-------------------

NUMAPROF was not yet testing on multi-million line application so we expect some slow down on such big code.
But it should be able to work. Although, the web GUI might lag due to too much data. In this case, enable
filtering option at profiling time by using option to remove all entries smaller than 0.2% from the output profile:

```
numaprof-pintool -o output:removeSmall=true,output:removeRatio=0.2 ./benchmark --my-option
```

Licence
-------

Numaprof is distributed under CeCILL-C licence which is LGPL compatible.
