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

char message_buff[100];

void callback(char* topic, byte* payload, unsigned int length);
void (*clientCallback)(String message);

//create our mqtt client object, params are server, port, callback and wifi client
PubSubClient client(MQTT_SERVER, 1883, callback, wifi);

// implement our callback method thats called on receiving data
void callback(char* topic, byte* payload, unsigned int length) {
    //store the received payload and print it at the Serial port
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = '\0';
    String message(p);
    (*clientCallback)(message);
    Serial.println("Command Received: "+message);
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
  wifi_connect(ssid, password);

  mqtt_connect(); 
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

void RelayrClient::connectClient(void (*clientCb)(String)) {
  clientCallback = clientCb;
}
