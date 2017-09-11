Numaprof
========

What is it ?
------------

Numaprof is currently a prototype to implement a NUMA memory 
profiler. The idea is to instrument the read/write operations in 
the application and check the NUMA location of the thread at 
accesss time to compare it to the memory location of the data.

I'm currently testing an implementation based on pintool but I'm 
also looking to possibly use a CLANG/LLVM plugin which offer
the same thing but without support on non recompiled code :
https://llvm.org/svn/llvm-project/safecode/trunk/lib/CommonMemorySafety/InstrumentMemoryAccesses.cpp

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

First download the last version of pintool (tested : 3.2-81205 on x86_64 arch : https://software.intel.com/en-us/articles/pin-a-binary-instrumentation-tool-downloads) and extract it somewhere.

Then use the configure script (for pintool mode do not use directly the cmake script, the configure script does extra things with pin):

```
mkdir build
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
numaprof-pintool ./benchmark --my-option
```

THe GUI password is currently fixed to admin/admin. You can then open the GUI : 

```
numaprof-webview numaprof-1234.json
```

If you run the webview on a remote node, you can forward the http session to your local browser by using :

```
ssh myhost -L8080:localhost:8080
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

Licence
-------

Numaprof is distributed under CeCILL-C licence which is LGPL compatible.
