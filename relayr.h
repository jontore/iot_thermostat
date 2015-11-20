#include <PubSubClient.h>
#include <ESP8266WiFi.h>

class RelayrClient {
  public:
    RelayrClient();
    void connect(char* ssid, char* password);
    void connectClient(void (*clientCb)(String));
    void publish(float temp);
};


