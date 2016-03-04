#include "isensor.h"
#include <DallasTemperature.h>

class DallasTempSensor : public ISensor 
{
  public:
    DallasTemperature* _sensors;
    DeviceAddress _address;
    float _temp;
    
    DallasTempSensor(DallasTemperature* sensors, DeviceAddress addr);
    static DallasTempSensor* create(DallasTemperature* sensors, byte index);
    
    virtual bool measure();
    virtual float getValue();
    virtual char *getId();
};
