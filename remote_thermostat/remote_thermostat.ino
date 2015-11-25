#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include "relayr.h"

#define SSID "lok"
#define PASSWORD "rem1000"


#define COOL_PIN 12
#define HEAT_PIN 2
#define DHTPIN 13

#define TEMPERATURE_CONTROL_RANGE 1
#define COMPRESSER_DELAY 2000

unsigned long lastTime = 0;
                                
int publishingPeriod = 1000;
int lastPublishTime = 0;
int lastCoolingTime = 0;

boolean cooling = false;
boolean heating = false;
float targetTemp = 32.0;


DHT dht;
RelayrClient remoteClient;

float readTemp() {
  return dht.getTemperature();
}

void cool() {
    lastCoolingTime = lastCoolingTime < 1 ? millis() : lastCoolingTime;
  
   if (millis() - lastCoolingTime > COMPRESSER_DELAY) {
     digitalWrite(HEAT_PIN, LOW);
     digitalWrite(COOL_PIN, HIGH);
     lastCoolingTime = millis();
   } 
}

void heat() {
  digitalWrite(HEAT_PIN, HIGH);
  digitalWrite(COOL_PIN, LOW);
  lastCoolingTime = 0;
}

void stop() {
  lastCoolingTime = 0;
  digitalWrite(HEAT_PIN, LOW);
  digitalWrite(COOL_PIN, LOW);
}

void control(float temp) {
  if (temp - TEMPERATURE_CONTROL_RANGE < targetTemp) {
    heating = true;
    cooling = false;
  } 
  if (temp + TEMPERATURE_CONTROL_RANGE > targetTemp) {
    heating = false;
    cooling = true;
  }
};

void setTargetTemp(float message) {  
  Serial.println("set target");
  targetTemp = message;
};

void setup() {
    Serial.begin(9600);
    
    pinMode(HEAT_PIN, OUTPUT);
    pinMode(COOL_PIN, OUTPUT);
    
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
