#ifndef __ISENSOR_H__
#define __ISENSOR_H__

#include <DallasTemperature.h>
#include "utils.h"
#include "stringbuilder.h"

class ISensor 
{
  protected:
    static char __id_buffer[32];
  
  public:
    ISensor* next;

    ISensor();

    virtual bool measure() { return (true); }    
    virtual void commit() {}

    virtual float getValue() { return 0; }
    virtual char *getId() = 0;
};


//DallasTemperature* DallasTempSensor::_sensors;

#endif
