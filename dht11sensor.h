#include "isensor.h"
#include "DHT.h"

class DHT11HumiditySensor : public ISensor 
{
  protected:
    byte _pin;
    float _hum;
    DHT _dht;

  public:

    DHT11HumiditySensor(byte pin);

    virtual bool measure();
    virtual float getValue();
    virtual char *getId();
};

