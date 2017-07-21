//Imported from libnuma to not have to link to libnuma inside pintool

#ifndef NUMAIF_H
#define NUMAIF_H 1

extern "C"
{

extern long move_pages(int pid, unsigned long count,void **pages, const int *nodes, int *status, int flags);
int getNumaOfPage(void * addr);

	
}

#endif //NUMAIF_H
