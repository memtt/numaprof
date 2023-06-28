/*****************************************************
             PROJECT  : numaprof
             VERSION  : 1.1.5
             DATE     : 06/2023
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

#ifndef CLOCK_HPP
#define CLOCK_HPP

/********************  HEADERS  *********************/
#include "../../../extern-deps/from-fftw/cycle.h"

/*******************  NAMESPACE  ********************/
namespace numaprof
{

/********************  TYPES  ************************/
typedef ticks ClockValue;

/********************  GLOBAS  **********************/
extern ClockValue gblClockRef;

/********************  CLASS  ***********************/
struct Clock
{
    static ClockValue get(void);
    static ClockValue get(ClockValue ref);
};

/*******************  FUNCTION  *********************/
inline ClockValue Clock::get(void)
{
    //manage ref
    if (gblClockRef == 0)
        gblClockRef = getticks();

    //get current
    ticks current = getticks();

    //take care of restart of clock
    if (current > gblClockRef)
        return current - gblClockRef;
    else
        return (((ticks)-1) - gblClockRef) + current;
}

/*******************  FUNCTION  *********************/
inline ClockValue Clock::get(ClockValue ref)
{
    return Clock::get() - ref;
}

}

#endif //CLOCK_HPP
