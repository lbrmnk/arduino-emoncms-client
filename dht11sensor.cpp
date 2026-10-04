#include "dht11sensor.h"
//DHT _dht(4, DHT11);

DHT11HumiditySensor::DHT11HumiditySensor(byte pin) : _dht(pin, DHT11)
{
  _pin = pin;
  _dht.begin();
}

bool DHT11HumiditySensor::measure()
{
  _hum = _dht.readHumidity();
  if (isnan(_hum)) {
    _hum = 0;  
    return false;
  }
  return true;
}


float DHT11HumiditySensor::getValue()
{
  return _hum; 
}

char *DHT11HumiditySensor::getId()
{
  strcpy(__id_buffer, "DHT11-hum-1");
  return __id_buffer;
}

