/*********************************************************************************************\
 * Auriol meteostation wireless sensor
 * 
 * source: https://github.com/Pirionfr/RFLink-Gateway/blob/master/Plugins/Plugin_045.c
 * 
 * Auriol Message Format: 
 * 
 * 1101 0110 1000 0000 1101 1111 1111 0000
 * AAAA AAAA BCCC DDDD DDDD DDDD EEEE FFFG 
 *
 * A = Rolling Code, no change during normal operation. (Device 'Session' ID) (Might also be 4 bits RC and 4 bits for channel number)
 * B = Battery status, 1=OK, 0=LOW
 * C = Always 000
 * D = Temperature (21.5 degrees is shown as decimal value 215, minus values have the high bit set and need to be subtracted from a base value of 4096)
 * E = Unknown
 * F = Unknown
 * G = sum of all bits xored together
 * 
 * Sample:
 * 20;34;DEBUG;Pulses=66;Pulses(uSec)=325,3725,325,1825,325,1825,325,1825,325,3700,325,3700,325,3700,325,3700,325,3700,325,1850,300,1825,325,1850,325,1825,325,1850,325,1825,300,1825,325,3725,300,3725,325,1825,325,1825,300,3725,300,1850,325,3725,300,1850,325,3725,300,3700,300,3725,300,1825,325,3700,325,3700,300,3700,325,1825,325;
 * 20;0A;DEBUG;Pulses=66;Pulses(uSec)=325,1850,300,1850,300,3700,300,1850,300,1850,300,1850,325,1850,300,1850,325,3700,325,1850,300,1850,300,1825,325,1850,300,1850,325,1825,300,1850,325,3725,300,3700,325,1825,300,1850,325,3700,300,3725,300,3725,300,1850,300,1850,300,3725,325,3700,300,1850,300,1825,325,1850,300,3700,300,1850,325;
 * 
 * 
 \*********************************************************************************************/
 
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

