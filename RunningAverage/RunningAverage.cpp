//
//    FILE: RunningAverage.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.2.08
//    DATE: 2015-apr-10
// PURPOSE: RunningAverage library for Arduino
//
// The library stores N individual values in a circular buffer,
// to calculate the running average.
//
// HISTORY:
// 0.1.00 - 2011-01-30 initial version
// 0.1.01 - 2011-02-28 fixed missing destructor in .h
// 0.2.00 - 2012-??-?? Yuval Naveh added trimValue (found on web)
//          http://stromputer.googlecode.com/svn-history/r74/trunk/Arduino/Libraries/RunningAverage/RunningAverage.cpp
// 0.2.01 - 2012-11-21 refactored
// 0.2.02 - 2012-12-30 refactored trimValue -> fillValue
// 0.2.03 - 2013-11-31 getElement
// 0.2.04 - 2014-07-03 added memory protection
// 0.2.05 - 2014-12-16 changed float -> double
// 0.2.06 - 2015-03-07 all size uint8_t
// 0.2.07 - 2015-03-16 added getMin() and getMax() functions (Eric Mulder)
// 0.2.08 - 2015-04-10 refactored getMin() and getMax() implementation
//
// Released to the public domain
//

#include "RunningAverage.h"
#include <stdlib.h>

RunningAverage::RunningAverage(uint8_t size)
{
    _size = size;
    _ar = (double*) malloc(_size * sizeof(double));
    if (_ar == NULL) _size = 0;
    clear();
}

RunningAverage::~RunningAverage()
{
    if (_ar != NULL) free(_ar);
}

// resets all counters
void RunningAverage::clear()
{
    _cnt = 0;
    _idx = 0;
    _sum = 0.0;
    _min = NAN;
    _max = NAN;
    for (uint8_t i = 0; i < _size; i++)
    {
        _ar[i] = 0.0; // keeps addValue simple
    }
}

// adds a new value to the data-set
void RunningAverage::addValue(double value)
{
    if (_ar == NULL) return;  // allocation error
    _sum -= _ar[_idx];
    _ar[_idx] = value;
    _sum += _ar[_idx];
    _idx++;
    if (_idx == _size) _idx = 0;  // faster than %
    // handle min max
    if (_cnt == 0) _min = _max = value;
    else if (value < _min) _min = value;
    else if (value > _max) _max = value;
    // update count as last otherwise if( _cnt == 0) above will fail
    if (_cnt < _size) _cnt++;
}

// returns the average of the data-set added sofar
double RunningAverage::getAverage()
{
    if (_cnt == 0) return NAN;
    return _sum / _cnt;
}

double RunningAverage::getDelta()
{
    if (_cnt==0) return NAN;
    
    double min=0; double max=0;
    for (uint8_t cx=0; cx<=_cnt; cx++) {

//Serial.print("  >");
//Serial.print(_ar[cx]);
      if (_ar[cx] <= min or min==0) { min = _ar[cx];};
      if (_ar[cx] >= max) { max = _ar[cx];};
    }
 
    return (max-min);
}

double RunningAverage::getDeltaPc()
{
     return 100*getDelta()/getAverage();
}




// returns the value of an element if exist, NAN otherwise
double RunningAverage::getElement(uint8_t idx)
{
    if (idx >=_cnt ) return NAN;
    return _ar[idx];
}

// fill the average with a value
// the param number determines how often value is added (weight)
// number should preferably be between 1 and size
void RunningAverage::fillValue(double value, uint8_t number)
{
    clear(); // TODO conditional?  if (clr) clear();

    for (uint8_t i = 0; i < number; i++)
    {
        addValue(value);
    }
}
// END OF FILE