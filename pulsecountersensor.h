#include "isensor.h"

class PulseCounterSensor : public ISensor 
{
  protected:
    volatile uint32_t* _counterPtr;
    int _id;

  public:
    PulseCounterSensor(int id, volatile uint32_t* counterPtr);

    virtual float getValue();
    virtual char *getId();
};

class DiffPulseCounterSensor : public PulseCounterSensor 
{
  protected:
    uint32_t _commitedValue;
    uint32_t _value;

  public:
    DiffPulseCounterSensor(int id, volatile uint32_t* counterPtr);

    virtual bool measure();
    virtual float getValue(); 
    virtual void commit();
};

class PowerPulseCounterSensor : public PulseCounterSensor
{
  public:
    volatile uint32_t* _lastPulseTimePtr;
    
    uint32_t _pulseTime;
    uint32_t _value;
    
    uint32_t _commitedValue;
    uint32_t _commitedTime;

    float _factor;

  public:

    PowerPulseCounterSensor(int id, float factor,
                            volatile uint32_t* counterPtr, 
                            volatile uint32_t* lastPulseTimePtr);

    virtual bool measure();
    virtual void commit();
    virtual float getValue();
};

