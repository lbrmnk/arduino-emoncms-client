#include <Arduino.h>
#include "receiver433.h"

Receiver433::Receiver433()
{
  pulsePrevTime = 0;
  pulseTime = 0;
  pulseStart = 0;
  pulseState = LOW;

  pulse = false;
  bitCount = 0;
}

void
Receiver433::begin(int pin)
{
  receivePin = pin;
}

void
Receiver433::isr()
{
  uint32_t utime = micros();
  int prevState = pulseState;
  pulseState = digitalRead(receivePin);
  if (pulseState == HIGH) {
    if (prevState == LOW) {
      pulseStart = utime;   
    }
  } else {
    if (pulseStart != 0 && (utime - pulseStart) > 180) {  
      pulsePrevTime = pulseTime;
      pulseTime = utime;
      pulse = true;
      pulseStart = 0;
    }
  }
}

bool
Receiver433::receive()
{
  isr();
  uint32_t utime = micros();

  // timeout
  if ((utime - pulseTime) > 10000 && matchCount > 3) {
    matchCount = 0;
    return true;
  }
  
  if (pulse) {
    int pulseWidth = pulseTime - pulsePrevTime;
    int pos = 1 + (bitCount / 8);
    if (bitCount >= 8 * (sizeof(data[0]) - 1)) {
      bitCount = 0;
    }

    if (pulseWidth > 3000) {
      data[0][0] = bitCount; 
      if (bitCount >= 24) {
        if (memcmp(data[0], data[1], sizeof(data[0])) == 0) {
          matchCount++;      
        } else {
          matchCount = 0;
          memcpy(data[1],data[0], sizeof(data[0]));
        }
      }  
      bitCount = 0;
      memset(data[0], 0, sizeof(data[0]));
    } else if (pulseWidth > 400) {        
      // logical 0
      data[0][pos] <<= 1;
      if (pulseWidth > 1500) {
        // logical 1
        data[0][pos] |= 1;
      }     
      bitCount++;
    }
    pulse = false;
  }

  return false;
}

int 
Receiver433::getBitCount()
{
  return data[1][0];
}

byte*
Receiver433::getData()
{
  return (data[1] + 1);
}

float
Receiver433::decodeTemp()
{
  int temp = 0;
  
  if (data[1][0] == 36) {
    temp = ((data[1][2] << 8) & 0x0f) | data[1][3];
  }
  if (data[1][0] == 24) {
    temp = (data[1][1] << 4) | (data[1][2] >> 4);
  }
  if (temp & 0x0800) { 
    temp |= 0xf000;
  }
  return (temp / 10.0);
}
