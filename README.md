# arduino-emoncms-client

Arduino heating monitoring client for emoncms.org website.

To run this you will need:

- Arduino dev board (UNO)
- Arduino IDE (http://arduino.cc/en/Main/Software)
- enc28j60 ethernet module (http://www.instructables.com/id/Add-Ethernet-to-any-Arduino-project-for-less-than-/)
- EtherCard library: https://github.com/jcw/ethercard
- OneWire library
- DallasTemperature library
- emoncms.org account and API key (http://emoncms.org)

Features:

- OneWire ds18b20 temperature sensor network
- Pulse counter (gas meter, electricity meter)
- Pulses to instant power conversion
- uses ping to detect ethernet module failure

Device connections

Enc28j60
  - CS - pin 10
  - SI - pin 11
  - SO - pin 12
  - SCK - pin 13
  - VCC - 3.3V
  - GND - GND
 
Onewire bus
  - DATA - pin 8 (with parasite power, 5V thru 4k7 resistor)
  - GND - GND
 
Hall sensor 
  - VCC - 5V (thru 50mA polyswitch)
  - GND - GND 
  - DATA - pin 2 (interrupt)
 
