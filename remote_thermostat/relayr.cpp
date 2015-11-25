#include "relayr.h"

RelayrClient::RelayrClient() {}

WiFiClient wifi;
   
//define your mqtt credentials
#define DEVICE_ID "d15931b6-02d6-4be7-aa53-5c62df13ea3e" 
#define MQTT_USER "d15931b6-02d6-4be7-aa53-5c62df13ea3e" 
#define MQTT_PASSWORD "5ztp5Wx19MXM"
#define MQTT_CLIENTID "T0VkxtgLWS+eqU1xi3xPqPg" //can be anything else
#define MQTT_TOPIC "/v1/d15931b6-02d6-4be7-aa53-5c62df13ea3e/"
#define MQTT_SERVER "mqtt.relayr.io"

#define DEBUG_PIN 14

char message_buff[100];

const String command = "set_temperature";
float commandValue;

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
  if (matchIndex != -1) {
    String value = msg.substring(39, 42);
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
  pinMode(DEBUG_PIN, OUTPUT);
  wifi_connect(ssid, password);

  while(!client.connected()) {
    mqtt_connect();
    delay(2000);
  }
}

void RelayrClient::publish(float temp) {
    //create our json payload
    String pubString = "{\"meaning\":\"temperature\", \"value\":";
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

void RelayrClient::connectClient(void (*clientCb)(float)) {
  clientCallback = clientCb;
  client.setCallback(callback);
  Serial.println("Connect client topic");
}

boolean RelayrClient::connected() {
  return client.connected();
}

void RelayrClient::loop() {
  client.loop();
}
