Change log
==========

master
------

 * Add a TLB cache to avoid walking in page table for every access.
 * Optionnaly handle access per batch, this provide a speed up of a factor 2 but loose in precision.

1.0.0 - 07/02/2018
------------------

 * Basic implementation of the base library to handle the mecanics of data taking.
 * Made pintool implementation
 * Provide basic pintool wrapper to ease usage
 * Basic support of callgrind format*
 * Add cutoff to not export values lower than a given value.
 * Add qt5 browser view
