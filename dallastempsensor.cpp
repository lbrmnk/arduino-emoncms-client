#include "dallastempsensor.h"
#include "utils.h"

DallasTempSensor::DallasTempSensor(DallasTemperature* sensors, int index) 
{
  _sensors = sensors;
  _index = index;  
  _temp = 0;
}

bool 
DallasTempSensor::measure() 
{
  _sensors->requestTemperaturesByIndex(_index);
  _temp = _sensors->getTempCByIndex(_index);
  if (_temp > -127) {
    return true;
  }
  return false;
}

float 
DallasTempSensor::getValue() 
{ 
  return _temp;
}

char *
DallasTempSensor::getId() 
{
  byte addr[8];      
  _sensors->getAddress(addr, _index);
  Utils::bytesToString(__id_buffer, addr, 8);
  return __id_buffer;
}

