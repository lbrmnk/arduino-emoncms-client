#include "isensor.h"
#include <DallasTemperature.h>
#include "utils.h"

class DallasTempSensor : public ISensor 
{
  protected:
    DallasTemperature* _sensors;
    DeviceAddress _address;
    float _temp;

  public:
    static DallasTempSensor* createByIndex(DallasTemperature* sensors, byte index);
    static char* addressToString(DeviceAddress addr);
    static char* addressToString(char *buf, DeviceAddress addr);

    DallasTempSensor(DallasTemperature* sensors, DeviceAddress addr);

    virtual bool measure();
    virtual float getValue();
    virtual char *getId();
};
