#include "dallastempsensor.h"
#include "utils.h"

DallasTempSensor::DallasTempSensor(DallasTemperature* sensors, DeviceAddress addr) 
{
  _sensors = sensors;
  _temp = 0;

  memcpy(_address, addr, sizeof(DeviceAddress));
}

DallasTempSensor*
DallasTempSensor::createByIndex(DallasTemperature* sensors, byte index)
{
  DeviceAddress addr;      
  sensors->getAddress(addr, index);
  return new DallasTempSensor(sensors, addr);
}

 char* 
DallasTempSensor::addressToString(DeviceAddress addr) {
  Utils::bytesToString(__id_buffer, addr, 8);
  return __id_buffer;
}

bool 
DallasTempSensor::measure() 
{
  // _sensors->requestTemperaturesByIndex(_index);
  if (_sensors->requestTemperaturesByAddress(_address)) {
    int raw = _sensors->getTemp(_address);
    if (raw != DEVICE_DISCONNECTED_RAW) {
      _temp = DallasTemperature::rawToCelsius(raw);
      return true;
    }
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
  return addressToString(_address);
}

