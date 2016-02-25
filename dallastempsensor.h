#include "isensor.h"

class DallasTempSensor : public ISensor 
{
  public:
    DallasTemperature* _sensors;
    int _index;
    float _temp;
    
    DallasTempSensor(DallasTemperature* sensors, int index);

    virtual bool measure();
    virtual float getValue();
    virtual char *getId();
};
