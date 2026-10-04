// Ardiono emoncms.org client
// author: lbrmnk, 2016
// http://opensource.org/licenses/mit-license.php

/*---------------------------------------------------------------------------*/
//
// ethernet configuration
// uncomment your configuration
//
// #define ETHERNET_TYPE 0 // Wiznet W5100 ethenet shield and Ethernet library
// #define ETHERNET_TYPE 1 // ENC28J60 and UIPEthernet library
// #define ETHERNET_TYPE 2 // ENC28J60 and EtherCard library

#define ETHERNET_TYPE 2

#define LED_YELLOW    6
#define LED_RED       7
//
/*---------------------------------------------------------------------------*/

// include right ethernet library
//

#if ETHERNET_TYPE == 2
  #define USE_ETHERCARD
  #define USE_ENC28J60
  #include <EtherCard.h>  

  #define ETHERNET_BUFSIZE 512
  byte Ethernet::buffer[ETHERNET_BUFSIZE];
#else
  #if ETHERNET_TYPE == 1
    #define USE_ENC28J60
    #include <UIPEthernet.h>   // ENC28J60 compatibility library
  #else
    #define USE_ETHERNETSHIELD
    #include <Ethernet.h>      // Wiznet W5100 ethenet shield
  #endif
  EthernetClient client;
#endif

#include <DallasTemperature.h>
#include <LiquidCrystal.h>

#include "isensor.h"
#include "dallastempsensor.h"
#include "pulsecountersensor.h"
#include "utils.h"
#include "stringbuilder.h"
#include "receiver433.h"
#include "dht11sensor.h"

#include "emoncmsconfig.h" // edit site information here!

#define ETHERCARD_PIN 10
#define ONEWIREBUS_PIN 8
#define ONEWIREBUS2_PIN 4
#define PULSEINTERRUPT_PIN 2
#define ETHERCARD_RESET_PIN 5
#define RECEIVER_PIN 9

OneWire  ds(ONEWIREBUS_PIN);  // Connect your 1-wire device to pin 8
OneWire  ds2(ONEWIREBUS2_PIN);  // Connect your 1-wire device to pin 8

DallasTemperature sensors(&ds);
DallasTemperature sensors2(&ds2);

Receiver433 rx;
LiquidCrystal lcd(A5, A4, A3, A2, A1, A0);


const char website[] PROGMEM = WEBSITE; //"emoncms.org";

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 }; 

static volatile uint32_t lastUpdate = 0;
static volatile uint32_t pulseCount = 0;
static volatile uint32_t last_interrupt_time_hi = 0;
static volatile uint32_t last_interrupt_time = 0;

ISensor* sensorList = NULL;
ISensor* currentSensor = NULL;
ISensor* commitSensor = NULL;
int      sensorCount = 0;
int      sensorPeriod = 20000;

void(* resetFn) (void) = 0;//declare reset funcrtion at address 0

void blinkLed(byte pin, int repeat, int onDelay, int offDelay) 
{
  for (int i = 0; i < repeat; i++) {
    digitalWrite(pin, HIGH);
    delay(onDelay);
    digitalWrite(pin, LOW);
    delay(offDelay);
  }
}

ISensor* findSensor(const char* sensorId)
{
  ISensor *tmp = sensorList;
  while (tmp != NULL) {
    if (strcmp(sensorId, tmp->getId()) == 0) { 
      return tmp;
    }
    tmp = tmp->next;
  }
  return NULL;
}

void addSensor(ISensor *sensor)
{
  char sensorId[32];
  if (sensor != NULL) {
    Serial.print(F("adding sensor..."));
    Serial.println(sensor->getId());
    sensor->next = sensorList;
    sensorList = sensor;

    sensorCount++;
  }
}

// called when a ping comes in (replies to it are automatic)
static void gotPinged (byte* ptr) 
{
#ifdef USE_ETHERCARD
  ether.printIp(F(">>> ping from: "), ptr);
#endif
}

void resetEthernet()
{
  Serial.print("reseting ethenet module... ");
  digitalWrite(ETHERCARD_RESET_PIN, LOW);
  delay(250);
  digitalWrite(ETHERCARD_RESET_PIN, HIGH);
  delay(250);
  Serial.println("DONE");
}

/****************************** SETUP ****************************/

#ifdef USE_ETHERCARD 

void setupEthernet()
{
  resetEthernet();
  Serial.println(F("setupEthernet() ... begin"));

  digitalWrite(LED_RED, HIGH);

  if (ether.begin(sizeof Ethernet::buffer, mymac, ETHERCARD_PIN) == 0) {
    Serial.println(F("Failed to access Ethernet controller"));
    resetFn();
  }
     
  Serial.println(F("setupEthernet() ... dhcp"));
  
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));

  ether.printIp(F("IP: "), ether.myip);
  ether.printIp(F("GW: "), ether.gwip);
  
  if (!ether.dnsLookup(website))
    Serial.println(F("DNS failed"));

  ether.printIp(F("SRV: "), ether.hisip);
    
  // call this to report others pinging us
  ether.registerPingCallback(gotPinged);

  digitalWrite(LED_RED, LOW);
}

#else 

void setupEthernet()
{
  Ethernet.begin(mymac);

  Serial.print(F("localIP: "));
  Serial.println(Ethernet.localIP());
  Serial.print(F("subnetMask: "));
  Serial.println(Ethernet.subnetMask());
  Serial.print(F("gatewayIP: "));
  Serial.println(Ethernet.gatewayIP());
  Serial.print(F("dnsServerIP: "));
  Serial.println(Ethernet.dnsServerIP());
}
#endif


// scans for 1wire devices.
// can be called multiple times.
//
int setupOneWire(DallasTemperature& sensors, OneWire& ds)
{
  int count = 0;
  sensors.begin();

  DeviceAddress addr;
  while(ds.search(addr)) {
    if (OneWire::crc8(addr, 7) == addr[7]) { // check crc to ensure valid address before adding to sensorList
      char sensorId[20];
      DallasTempSensor::addressToString(sensorId, addr);
      if (findSensor(sensorId) == NULL) {
        addSensor(new DallasTempSensor(&sensors, addr));
        count++;
      }
    } else {
      Serial.println(F("CRC is not valid!"));
    }    
  }

  blinkLed(6, count, 100, 350);

  // report parasite power requirements
  Serial.print(F("Parasite power is: ")); 
  Serial.println(sensors.isParasitePowerMode() ? F("ON") : F("OFF"));

  return count;
}

void setupDHT()
{
  DHT11HumiditySensor *dht = new DHT11HumiditySensor(4);
  addSensor(dht);
}

void setupCounters()
{
  addSensor(new PowerPulseCounterSensor(3, 36000 /*(float)3600000 * 0.01*/, &pulseCount, &last_interrupt_time));  
  addSensor(new DiffPulseCounterSensor(1, &pulseCount));
}

void setupLeds()
{
  pinMode(LED_YELLOW, OUTPUT); // LED 1
  pinMode(LED_RED, OUTPUT); // LED 2

  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_RED, HIGH);

  delay(500);

  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  delay(500);
}


void interruptHandler2();

void setupOneWireLCD(DallasTemperature& sensors, OneWire& ds)
{
    lcd.setCursor(0, 1);
    lcd.print(F("1wire..         "));
    int owDevCount = setupOneWire(sensors, ds);
    lcd.setCursor(8, 1);
    lcd.print(owDevCount);
    lcd.print(F("devs"));
}

void setup() {  
  Serial.begin(115200);
  Serial.println(F("\n[emoncms.org client]"));
  
  pinMode(ETHERCARD_RESET_PIN, OUTPUT);    // configure eth.module reset pin
  digitalWrite(ETHERCARD_RESET_PIN, HIGH); // set LOW to reset ethernet module, HIGH for normal operation

  lcd.begin(16, 2);
  lcd.print(F("emoncms client"));
  lcd.display();

  lcd.setCursor(0, 1);
  lcd.print(F("setup leds        "));
  setupLeds();

  //setupDHT();
  lcd.setCursor(0, 1);
  lcd.print(F("setup counters.."));
  setupCounters();

  setupOneWireLCD(sensors, ds);
  delay(2000);
  setupOneWireLCD(sensors2, ds2);
  delay(2000);

  //for (int i = 0; i < 2; i++) {
  //}

  lcd.setCursor(0, 1);
  lcd.print(F("setup ethernet    "));
  setupEthernet();
  
  lcd.setCursor(0, 1);
  for (int i = 0; i < 3; i++) {
    lcd.print(ether.myip[i]);
    lcd.print('.');
  }
  lcd.print(ether.myip[3]);
  delay(2000);

  rx.begin(RECEIVER_PIN);
  
  delay(1500);
  
  lastUpdate = millis();

  attachInterrupt(digitalPinToInterrupt(2), interruptHandler2, CHANGE);
  
  currentSensor = NULL;

  sensorPeriod = 1000 * (140 / (sensorCount + 1));

  Serial.print(F("Sensor count: "));
  Serial.print(sensorCount);
  Serial.print(F("Sensor Period: "));
  Serial.print(sensorPeriod);
  
  lcd.setCursor(0, 1);
  lcd.print(F("Period...       "));
  lcd.setCursor(10, 1);
  lcd.print(sensorPeriod);
  delay(1500);
  
  lcd.setCursor(0, 1);
  lcd.print(F("running...      "));
}

/****************************** interrupt counter  ****************************/

const int MIN_LOW_TIME = 2000;  // gas meter - minimum time between pulses
const int MIN_HIGH_TIME = 250;  // gas meter - minimum pulse duration (for debounce)

// handles interrupt in pin 2 from gas meter, 
// filters out random signals inducted to wires (like fluor tube on/off)
//
void interruptHandler2()
{
  unsigned long current_time = millis();
  int pin2 = digitalRead(2);
  digitalWrite(6, pin2); // signal LED to show current input state
  
  if (pin2 == HIGH) {
    if ((current_time - last_interrupt_time) > MIN_LOW_TIME) {
      // pulse start, only if there were MIN_LOW_TIME interval since last pulse
      last_interrupt_time_hi = current_time;
    }
  } else { // LOW
    if (last_interrupt_time_hi != 0 && (current_time - last_interrupt_time_hi) > MIN_HIGH_TIME) {
      // measure pulse duration, accept only pulse longer than MIN_HIGH_TIME
      last_interrupt_time = current_time;
      pulseCount++;
    }
    last_interrupt_time_hi = 0;
  }
}

void led_show_dashboard(const char* data, word len)
{
  int i = 4;
  while (i < len) {
    if (strncmp(data + i - 4, "\r\n\r\n", 4) == 0) {
      Serial.print(F("led_show_dashboard: "));      
      Serial.println(data + i);

      lcd.clear();
      lcd.print(data + i);
      lcd.setCursor(0, 1);
      lcd.print(data + i + 16);
      break;
    }
    i++;
  }
}

// called after successfull http request
// updates "lastUpdate" timer to see that website and internet connection is up.
void
uploadCallback(byte status, word off, word len) 
{
  Serial.println(F("uploadCallback initiated"));
  Serial.print(F("Status: ")); Serial.println(status);

  Ethernet::buffer[ETHERNET_BUFSIZE - 1] = 0;
  /*
  Serial.print("off   : "); Serial.println(off);
  Serial.print("len   : "); Serial.println(len);
  Serial.println(">>>");
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
  */
  led_show_dashboard((const char*) Ethernet::buffer + off, len);
    
  if (commitSensor != NULL) {
    Serial.print(F("commiting sensor: "));
    Serial.println(commitSensor->getId());
    commitSensor->commit();
  }
  
  lastUpdate = millis();
  digitalWrite(7, LOW); // set upload progress signal LED off
}

// String query_string;
char buf[128];
StringBuilder query_string(buf, sizeof(buf));


// builds URL and sends http request to emoncmd.org website input handler (see input api help http://emoncms.org/input/api)
int
uploadSensorValue(ISensor *sensor)
{
  // http://emoncms.org/input/post.json?json={28FFC8D971150373:22}&apikey=8aa52f41e15069b7f3ddd39b7e72be7d
  query_string.clear();
  query_string.append("node=0&json=%7B");
  // sensor->appendId(query_string);
  query_string.append(sensor->getId());
  query_string.append(':');
  // sensor->appendValue(query_string);
  query_string.append(sensor->getValue());
  query_string.append("%7D&apikey=");
  query_string.append(APIKEY);

  /*
  lcd.clear();
  lcd.print(sensor->getId());
  lcd.setCursor(0, 1);
  lcd.print(sensor->getValue());
  */

  Serial.println(query_string.c_str());
  Serial.print("calling ether.browseUrl() ... ");
 
  digitalWrite(7, HIGH); // light up LED to show that upload is in progress
  commitSensor = sensor;

#ifdef USE_ETHERCARD 
  // ether.browseUrl(PSTR("/input/post.json?"), query_string.c_str(), website, uploadCallback);
  ether.browseUrl(PSTR("/input-dash.php?"), query_string.c_str(), website, uploadCallback);
#else
  // client.stop();
  
  if (client.connected() || client.connect(WEBSITE, 80)) {
    Serial.println(F("http client connected."));
    client.print(F("GET /input/post.json?"));
    client.print(query_string.c_str());
    client.println(F(" HTTP/1.1"));

    client.print(F("Host: ")); 
    client.println(WEBSITE);
    
    // client.println(F("Connection: close"));
    client.println(F("Connection: keep-alive"));
    client.println();
  } else {
    Serial.println(F("http connection failed."));
  }
#endif
}
    
/****************************** Functions ****************************/

Sensor rxSensor;

void loop () {
  static uint32_t lastCount = 0;
  static uint32_t pingTimer = 0;
  static uint32_t lastPingReply = 0;

#ifdef USE_ETHERCARD 
  word len = ether.packetReceive(); // go receive new packets
  word pos = ether.packetLoop(len); // respond to incoming pings

  // handle 433 wireless sensors.
  if (rx.receive()) {
    float temp = rx.decodeTemp();
    if (temp > -50 && temp < 50) {
      char buf[8];
      sprintf(buf, "rf_%d", rx.getBitCount());
      rxSensor.setId(buf);
      rxSensor.setValue(temp);
      uploadSensorValue(&rxSensor);
    }
  }

  // handle ping response
  if (len > 0 && ether.packetLoopIcmpCheckReply(ether.gwip)) {
    Serial.print("  ");
    Serial.print((millis() - pingTimer), 3);
    Serial.println(" ms");
    lastPingReply = millis();
  }
#else 
  if (client.available()) {
    digitalWrite(7, LOW);
    lastUpdate = millis();
    
    char c = client.read();
    Serial.write(c);
  }
  lastPingReply = millis();
#endif

  // debug output for pulse counter
  if (lastCount != pulseCount) {
    Serial.print("pulse ... ");
    Serial.println(pulseCount);
    lastCount = pulseCount;
  }

#ifdef USE_ETHERCARD 
  // timer for ping test, every 20s
  if (((millis() % sensorPeriod) - sensorPeriod/2) == 0) {
    ether.printIp("Pinging: ", ether.gwip);
    pingTimer = millis();
    ether.clientIcmpRequest(ether.gwip);
  }
#endif

  // main measure routing, every 20s
  if ((millis() % sensorPeriod) == 0) {

    if (currentSensor != NULL) {
      // if has any sensor to measure, go on...
        
      Serial.print(F("Requesting sensor: "));
      Serial.print(currentSensor->getId());
      Serial.print(F(" ... "));

      // request sensor value
      if (currentSensor->measure()) {
        // when a valid measure, then output value to website
        Serial.println(currentSensor->getValue());
        uploadSensorValue(currentSensor);
      } else {
        Serial.println(F("false")); 
      }

      currentSensor = currentSensor->next;    
    } else {
      currentSensor = sensorList;    
    }

    // ensure that network and website is up
    if (millis() > (lastPingReply + (2L * 60000L)) ||  // no ping to gateway for 2 minutes  or
        millis() > (lastUpdate + (15L * 60000L))) {    // no upload to website for 15 minutes
      Serial.print("reseting...");
      delay(100);
      resetFn(); // restart arduino
      lastUpdate = millis();
    }
  }
}
