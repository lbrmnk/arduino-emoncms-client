// Ping a remote server, also uses DHCP and DNS.
// 2011-06-12 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <EtherCard.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "isensor.h"
#include "dallastempsensor.h"
#include "pulsecountersensor.h"
#include "emoncmsconfig.h"

#define ETHERCARD_PIN 10
#define ONEWIREBUS_PIN 8
#define PULSEINTERRUPT_PIN 2
#define ETHERCARD_RESET_PIN 5


OneWire  ds(ONEWIREBUS_PIN);  // Connect your 1-wire device to pin 8
DallasTemperature sensors(&ds);

const char website[] PROGMEM = WEBSITE; //"emoncms.org";

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 }; 
byte Ethernet::buffer[512];

static volatile uint32_t lastUpdate = 0;
static volatile uint32_t pulseCount = 0;
static volatile uint32_t last_interrupt_time_hi = 0;
static volatile uint32_t last_interrupt_time = 0;

ISensor* sensorList = NULL;
ISensor *currentSensor = NULL;

void(* resetFn) (void) = 0;//declare reset funcrtion at address 0

void addSensor(ISensor *sensor)
{
  Serial.println(F("adding sensor..."));
  sensor->next = sensorList;
  sensorList = sensor;
}

// called when a ping comes in (replies to it are automatic)
static void gotPinged (byte* ptr) 
{
  ether.printIp(F(">>> ping from: "), ptr);
}

void resetEthernet()
{
  digitalWrite(ETHERCARD_RESET_PIN, LOW);
  delay(1);
  digitalWrite(ETHERCARD_RESET_PIN, HIGH);
  delay(50);
}

/****************************** SETUP ****************************/
void setupEthernet()
{
  resetEthernet();

  Serial.println(F("setupEthernet() ... begin"));
  
  if (ether.begin(sizeof Ethernet::buffer, mymac, ETHERCARD_PIN) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
     
  Serial.println(F("setupEthernet() ... dhcp"));
  
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));

  ether.printIp(F("IP: "), ether.myip);
  ether.printIp(F("GW: "), ether.gwip);
  
#if 1
  // use DNS to locate the IP address we want to ping
  if (!ether.dnsLookup(website))
    Serial.println(F("DNS failed"));
#else
  ether.parseIp(ether.hisip, "192.168.111.102");
  // memcpy(ether.hisip, ether.gwip, sizeof(ether.hisip));
#endif
  ether.printIp(F("SRV: "), ether.hisip);
    
  // call this to report others pinging us
  ether.registerPingCallback(gotPinged);
}

void setupOneWire()
{
  sensors.begin();
  int sensorsCount = sensors.getDeviceCount();
    
  // locate devices on the bus
  Serial.print(F("Locating devices..."));
  Serial.print(F("Found "));
  Serial.print(sensorsCount, DEC);
  Serial.println(F(" devices."));

  // DallasTempSensor::begin(&sensors);
  for (int i = sensorsCount - 1; i >= 0; i--) {
    addSensor(new DallasTempSensor(&sensors, i));
  }

  // report parasite power requirements
  Serial.print(F("Parasite power is: ")); 
  Serial.println(sensors.isParasitePowerMode() ? F("ON") : F("OFF"));
}

void setupCounters()
{
  addSensor(new PowerPulseCounterSensor(3, 36000 /*(float)3600000 * 0.01*/, &pulseCount, &last_interrupt_time));  
  addSensor(new DiffPulseCounterSensor(1, &pulseCount));
  // addSensor(new PulseCounterSensor(2, &pulseCount));
}

void interruptHandler();
void interruptHandler2();

void setup () {  
  pinMode(ETHERCARD_RESET_PIN, OUTPUT);
  digitalWrite(ETHERCARD_RESET_PIN, HIGH);

  Serial.begin(57600);
  Serial.println(F("\n[emoncms.org client]"));

  setupEthernet();
  setupCounters();
  setupOneWire();

  lastUpdate = millis();

  attachInterrupt(digitalPinToInterrupt(2), interruptHandler2, CHANGE);
  
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);

  digitalWrite(7, LOW);
  currentSensor = sensorList;
}

/****************************** interrupt counter  ****************************/

const int MIN_LOW_TIME = 2000;
const int MIN_HIGH_TIME = 250;

void interruptHandler2()
{
  unsigned long current_time = millis();
  int pin2 = digitalRead(2);
  digitalWrite(6, pin2);
  if (pin2 == HIGH) {
    if ((current_time - last_interrupt_time) > MIN_LOW_TIME) {
      last_interrupt_time_hi = current_time;
    }
  } else { // LOW
    if (last_interrupt_time_hi != 0 && (current_time - last_interrupt_time_hi) > MIN_HIGH_TIME) {
      last_interrupt_time = current_time;
      pulseCount++;
    }
    last_interrupt_time_hi = 0;
  }
}

void debounceHandler()
{
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 3000ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 3000) {
    last_interrupt_time = interrupt_time;
    interruptHandler();
  }
}

void
uploadCallback(byte status, word off, word len) 
{
  Serial.println(F("uploadCallback initiated"));
  Serial.print(F("Status: ")); Serial.println(status);

  /*
  Serial.print("off   : "); Serial.println(off);
  Serial.print("len   : "); Serial.println(len);
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
  */
  if (currentSensor != NULL) {
    Serial.print(F("commiting sensor: "));
    Serial.println(currentSensor->getId());
    currentSensor->commit();
  }
  
  lastUpdate = millis();
  digitalWrite(7, LOW);
}

// String query_string;
char buf[128];
StringBuilder query_string(buf, sizeof(buf));

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

  Serial.println(query_string.c_str());
  //ether.browseUrl(PSTR("/input/post.json"), query_string.c_str(), website, uploadCallback);
  
  Serial.print("calling ether.browseUrl() ... ");
 
  digitalWrite(7, HIGH);

  ether.browseUrl(PSTR("/input/post.json?"), 
                       query_string.c_str(),  
                       website, 
                       uploadCallback);
  
}
    
/****************************** Functions ****************************/

void loop () {
  static uint32_t lastCount = 0;
  static uint32_t pingTimer = 0;
  static uint32_t lastPingReply = 0;
  
  word len = ether.packetReceive(); // go receive new packets
  word pos = ether.packetLoop(len); // respond to incoming pings
  
  if (len > 0 && ether.packetLoopIcmpCheckReply(ether.gwip)) {
    Serial.print("  ");
    Serial.print((millis() - pingTimer), 3);
    Serial.println(" ms");
    lastPingReply = millis();
  }

  if (lastCount != pulseCount) {
    Serial.print("pulse ... ");
    Serial.println(pulseCount);
    lastCount = pulseCount;
  }
  
  if (((millis() % 20000) - 10000) == 0) {
    ether.printIp("Pinging: ", ether.gwip);
    pingTimer = millis();
    ether.clientIcmpRequest(ether.gwip);
  }
  
  if ((millis() % 20000) == 0) {

    if (currentSensor == NULL) {
      currentSensor = sensorList;
    } else {
      currentSensor = currentSensor->next;    
    }

    if (currentSensor != NULL) {
        
      Serial.print(F("Requesting sensor: "));
      Serial.print(currentSensor->getId());
      Serial.print(F(" ... "));
      if (currentSensor->measure()) {
        Serial.println(currentSensor->getValue());
        uploadSensorValue(currentSensor);
      } else {
        Serial.println(F("false")); 
      }
    }

    if (millis() > (lastPingReply + (2L * 60000L)) ||  // 2 minuty bez pingu na branu
        millis() > (lastUpdate + (15L * 60000L))) {    // 15 minut bez uploadu na web
      Serial.print("reseting...");
      delay(100);
      resetFn();
      lastUpdate = millis();
    }
  }
}
