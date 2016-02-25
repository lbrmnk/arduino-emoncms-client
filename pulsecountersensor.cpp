#include "pulsecountersensor.h"
    
PulseCounterSensor::PulseCounterSensor(int id, volatile uint32_t* counterPtr) 
{
  _counterPtr = counterPtr;
  _id = id;
}

float 
PulseCounterSensor::getValue() 
{ 
  return (float)*_counterPtr;
}

char *
PulseCounterSensor::getId() 
{
  sprintf(__id_buffer, "counter%d", _id);
  return __id_buffer;
}

/************************ DiffPulseCounterSensor *******************************/

DiffPulseCounterSensor::DiffPulseCounterSensor(int id, volatile uint32_t* counterPtr) 
  : PulseCounterSensor(id, counterPtr) 
{
  _value = 0;
  _commitedValue = 0;
}

bool 
DiffPulseCounterSensor::measure() 
{
  _value = *_counterPtr;
  return (true);
}
    
float 
DiffPulseCounterSensor::getValue() 
{ 
  return (float)(_value - _commitedValue);
}

void 
DiffPulseCounterSensor::commit() {
  _commitedValue = _value;
}

/************************* PowerPulseCounterSensor ****************************/

PowerPulseCounterSensor::PowerPulseCounterSensor(int id, float factor, 
                                                 volatile uint32_t* counterPtr, 
                                                 volatile uint32_t* lastPulseTimePtr) 
   : PulseCounterSensor(id, counterPtr) 
{
  _lastPulseTimePtr = lastPulseTimePtr;

  _value = 0;
  _pulseTime = 0; 

  _commitedValue = 0;
  _commitedTime  =0;

  _factor = factor;
}

bool 
PowerPulseCounterSensor::measure() 
{
  _value = *_counterPtr;
  _pulseTime = *_lastPulseTimePtr;

  return (true);

  /*  
  Serial.println(F("PowerPulseCounterSensor::measure"));
  Serial.print(F("_value: "));
  Serial.println(_value);
  Serial.print(F("_commitedValue: "));
  Serial.println(_commitedValue);
  
  if ((_value == _commitedValue) ||              // vysilat kdyz: je nula
      (_value - _commitedValue) >= 5 ||          // peak vic jak 5 pulsu
      (_pulseTime - _commitedTime) > 3*60*1000L)  // dlouho se nic neposlalo
  {
    return (true);
  }
  
  return false;
  */
}

void 
PowerPulseCounterSensor::commit() 
{
  _commitedValue = _value;
  _commitedTime = _pulseTime;
}

float 
PowerPulseCounterSensor::getValue() { 
  if (_pulseTime == _commitedTime) return 0;
  return _factor * (float)(_value - _commitedValue) / (float)(_pulseTime - _commitedTime);
}


