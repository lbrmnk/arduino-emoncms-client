# arduino-emoncms-client

Arduino home automation client for emoncms.org website.

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

Arduino pin connections

