#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <EEPROM.h>
#include "relayr.h"

#define SSID "ssid"
#define PASSWORD "password"

#define DEFAULT_TARGET_TEMP 19

#define COOL_PIN 13
#define HEAT_PIN 14
#define DHTPIN 12

#define TEMPERATURE_CONTROL_RANGE 1
#define COMPRESSER_DELAY 20000

unsigned long lastTime = 0;
                                
int publishingPeriod = 1000;
int lastPublishTime = 0;
int lastCoolingTime = 0;

boolean cooling = false;
boolean heating = false;
float targetTemp = 0;
int eeAddress = 0;


DHT dht;
RelayrClient remoteClient;

float readTemp() {
  return dht.getTemperature();
}

void cool() {
    lastCoolingTime = lastCoolingTime < 1 ? millis() : lastCoolingTime;
  
   if (millis() - lastCoolingTime > COMPRESSER_DELAY) {
     digitalWrite(COOL_PIN, LOW);
     lastCoolingTime = millis();
   } 
   
   digitalWrite(HEAT_PIN, HIGH);
}

void heat() {
  digitalWrite(HEAT_PIN, LOW);
  digitalWrite(COOL_PIN, HIGH);
  lastCoolingTime = 0;
}

void stop() {
  lastCoolingTime = 0;
  digitalWrite(HEAT_PIN, HIGH);
  digitalWrite(COOL_PIN, HIGH);
}

void control(float temp) {
  if (temp + TEMPERATURE_CONTROL_RANGE < targetTemp) {
    heating = true;
    cooling = false;
  } else if (temp - TEMPERATURE_CONTROL_RANGE > targetTemp) {
    heating = false;
    cooling = true;
  } else {
    heating = false;
    cooling = false;
  }
};

void setTargetTemp(float message) {  
  Serial.println("set target");
  targetTemp = message;
  EEPROM.put(eeAddress, targetTemp);
  EEPROM.commit();
};

void setup() {
    Serial.begin(9600);
    EEPROM.begin(512);

    if (targetTemp == 0) {
      EEPROM.get(eeAddress, targetTemp);
      targetTemp = targetTemp == NAN ? DEFAULT_TARGET_TEMP : targetTemp;
    }
    
    pinMode(HEAT_PIN, OUTPUT);
    pinMode(COOL_PIN, OUTPUT);
    digitalWrite(HEAT_PIN, HIGH);
    digitalWrite(COOL_PIN, HIGH);
    
    publishingPeriod = publishingPeriod > 200 ? publishingPeriod : 200;

    remoteClient.connect(SSID, PASSWORD);

    dht.setup(DHTPIN, DHT::DHT22);

    remoteClient.connectClient(&setTargetTemp);
}

void loop() {
  float temp = readTemp();

  control(temp);

  if (heating) {
    heat();
  } else if (cooling) {
    cool();
  } else {
    stop();
  }

  if(remoteClient.connected()) {
    remoteClient.loop();
    
    if (millis() - lastPublishTime > publishingPeriod) {
      lastPublishTime = millis();
      remoteClient.publish(temp);
    }
  }

  delay(2000);
}
