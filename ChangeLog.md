Change log
==========

1.1.2 - 14/11/2020
------------------

 * Fix issue with ALSR & fPIE on new linux distribution like debian/ubuntu

1.1.1 - 14/11/2020
------------------

 * Fix python 2->3 issue forgoten before release

1.1.0 - 01/11/2020
------------------

 * Now handle local/remote MCDRAM access instead of just MCDRAM.
 * Add instruction cache to go faster
 * Add a TLB cache to avoid walking in page table for every access.
 * Optionnaly handle access per batch, this provide a speed up of a factor 2 but loose in precision.
 * Add cache simulation with a simple L1 per thread model with LRU replacement policy. Disabled by default.
 * Total rewrite of the ./configure infrastructure to be more reusable in other projects.
 * Move GUI server to python3.
 * Support version 3.16 of pintool.

1.0.0 - 07/02/2018
------------------

 * Basic implementation of the base library to handle the mecanics of data taking.
 * Made pintool implementation
 * Provide basic pintool wrapper to ease usage
 * Basic support of callgrind format*
 * Add cutoff to not export values lower than a given value.
 * Add qt5 browser view
