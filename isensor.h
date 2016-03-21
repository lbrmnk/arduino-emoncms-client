#ifndef __ISENSOR_H__
#define __ISENSOR_H__

#include <stdint.h>
#include <string.h>

// common interface for any sensor
// 
// - call measure to acquire sensor value
// - use getValue and getId to post sensor state to the website
// - use commit to acknowledge that sensor value was uploaded to the website
class ISensor 
{
  protected:
    // shared buffer for sensor name, used by getId method
    static char __id_buffer[32];
  
  public:
    // daisy chain sensors to a simple list
    ISensor* next;

    // ctor
    ISensor();

    // request sensor for value, returns true when a valid value measured
    virtual bool measure() { return (true); }    

    // acknowledges that measured value was uploaded, usefull for various counters
    virtual void commit() {}

    // return measures value
    virtual float getValue() { return 0; }

    // returs sensor unique name
    virtual char *getId() = 0;

    static char *getBuffer() { return __id_buffer; }
};

class Sensor : public ISensor 
{
  public:
    float _value;

    void setValue(float value) { _value = value; }
    float getValue() { return _value; }

    void setId(char *id) { strncpy(__id_buffer, id, sizeof(__id_buffer)); }
    virtual char *getId() { return __id_buffer; }
};


//DallasTemperature* DallasTempSensor::_sensors;

#endif
