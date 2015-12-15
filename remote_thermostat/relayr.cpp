#include "relayr.h"

RelayrClient::RelayrClient() {}

WiFiClient wifi;

            
//define your mqtt credentials
#define DEVICE_ID "5abd9c5e-8ba2-4cd4-8695-a2debf8fab4b" 
#define MQTT_USER "5abd9c5e-8ba2-4cd4-8695-a2debf8fab4b" 
#define MQTT_PASSWORD "MQTT_PWD"
#define MQTT_CLIENTID "TWr2cXouiTNSGlaLev4+rSw" //can be anything else
#define MQTT_TOPIC "/v1/5abd9c5e-8ba2-4cd4-8695-a2debf8fab4b/"
#define MQTT_SERVER "mqtt.relayr.io"


char message_buff[100];

const String command = "set_temperature";
const String valueKey = "value\":\"";

void callback(char* topic, byte* payload, unsigned int length);
void handlePayload(char* message);
void (*clientCallback)(float message);

PubSubClient client(MQTT_SERVER, 1883, wifi);

// implement our callback method thats called on receiving data
void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = 0;
  String message(p);
  //print the topic and the payload received
  Serial.println("topic: " + String(topic));
  Serial.println("payload: " + message);
  
  handlePayload(p);
}

void handlePayload(char* message) {  
  String msg = String(message);

  int matchIndex = msg.indexOf(command);
  int valueIndex = msg.indexOf(valueKey);
  if (matchIndex != -1 && valueIndex != -1) {
    int valuePosition = valueIndex + valueKey.length();
    String value = msg.substring(valuePosition, valuePosition + 2);
    char floatbuf[4];
    value.toCharArray(floatbuf, sizeof(floatbuf));
    float setTemp = atof(floatbuf);
    (*clientCallback)(setTemp);
  }
}


void wifi_connect(char* ssid, char* password) {
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void mqtt_connect() {
  Serial.println("Connecting to mqtt server");
  if(client.connect(MQTT_CLIENTID, MQTT_USER, MQTT_PASSWORD)) {
    Serial.println("Connection success, subscribing to topic");
    //subscribe to a topic, cmd in this case
    client.subscribe("/v1/" DEVICE_ID "/cmd");
  }
  else {
    Serial.println("Connection failed, check your credentials");
  }
}

void RelayrClient::connect(char* ssid, char* password) {
  if (WiFi.status() != WL_CONNECTED) {
    wifi_connect(ssid, password);
  }

  while(!client.connected()) {
    mqtt_connect();
    delay(2000);
  }
}

void publish(float temp, String command) {
    //create our json payload
    String pubString = "{\"meaning\":\"" + command + "\", \"value\":";
    //read and add sensor data to payload
    char tmp[20];
    dtostrf(temp, 4, 3, tmp);
    pubString += tmp;
    pubString += "}";
    pubString.toCharArray(message_buff, pubString.length()+1);
    //publish our json payload
    client.publish("/v1/" DEVICE_ID "/data", message_buff);
    Serial.println("Publishing " + String(message_buff));

}

void RelayrClient::publishTemperature(float temp) {
  publish(temp, "temperature");
};

void RelayrClient::publishSetTemperature(float temp) {
  publish(temp, "set_temperature");
};

void RelayrClient::connectClient(void (*clientCb)(float)) {
  clientCallback = clientCb;
  client.setCallback(callback);
  Serial.println("Connect client topic");
}

boolean RelayrClient::connected() {
  return client.connected() && WiFi.status() == WL_CONNECTED;
}

void RelayrClient::loop() {
  client.loop();
}
