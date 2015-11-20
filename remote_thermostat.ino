#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include "relayr.h"

#define SSID "wifi"
#define PASSWORD "my_password"


#define COOL_PIN 3
#define HEAT_PIN 2
#define DHTPIN 7

#define TEMPERATURE_CONTROL_RANGE 2
#define COMPRESSER_DELAY 20000

unsigned long lastTime = 0;
                                
int publishingPeriod = 1000;
int lastPublishTime = 0;
int lastCoolingTime = 0;

boolean cooling = false;
boolean heating = false;
float targetTemp = 0.0;


DHT dht;
RelayrClient remoteClient;

float readTemp() {
  return dht.getTemperature();
}

void cool() {
   if (millis() - lastCoolingTime > COMPRESSER_DELAY) {
     digitalWrite(COOL_PIN, HIGH);
     digitalWrite(HEAT_PIN, LOW);
   } else {
    lastCoolingTime = millis();
   }
}

void heat() {
  digitalWrite(HEAT_PIN, HIGH);
  digitalWrite(COOL_PIN, LOW);
  lastCoolingTime = 0;
}

void stop() {
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

void setTargetTemp(String message) {
  //TODO: Needs to be read from the JSON set temperature correctly
  targetTemp = message.toFloat();
};

void setup() {
    //set 200ms as minimum publishing period
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
  
  if (millis() - lastPublishTime > publishingPeriod) {
    lastPublishTime = millis();
    remoteClient.publish(temp);
  }

  delay(2000);
}
