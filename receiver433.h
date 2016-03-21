#include <stdint.h>

class Receiver433
{
  private: 
    byte receivePin;
    byte bitCount;
    byte matchCount;
    volatile bool pulse;    
    volatile int pulseState;
    volatile uint32_t pulsePrevTime;
    volatile uint32_t pulseTime;
    volatile uint32_t pulseStart;

    byte data[2][9];

    void isr();

  public:
    Receiver433();
    
    void begin(int pin);

    bool receive();

    bool dataAvailable();

    float decodeTemp();

    byte* getData();

    int   getBitCount();
};

