Change log
==========

master
------

 * Now handle local/remote MCDRAM access instead of just MCDRAM.
 * Add instruction cache to go faster
 * Add a TLB cache to avoid walking in page table for every access.
 * Optionnaly handle access per batch, this provide a speed up of a factor 2 but loose in precision.
 * Add cache simulation with a simple L1 per thread model with LRU replacement policy. Disabled by default.
 * Total rewrite of the ./configure infrastructure to be more reusable in other projects.

1.0.0 - 07/02/2018
------------------

 * Basic implementation of the base library to handle the mecanics of data taking.
 * Made pintool implementation
 * Provide basic pintool wrapper to ease usage
 * Basic support of callgrind format*
 * Add cutoff to not export values lower than a given value.
 * Add qt5 browser view
