#include <WiFi101.h>

#include "WolkConn.h"
#include "MQTTClient.h"

const char* ssid = "wifi_ssid";
const char* wifi_pass = "wifi_password";

const char *device_key = "device_key";
const char *device_password = "device_password";
const char* hostname = "api-demo.wolkabout.com";
int portno = 1883;

/* WolkConnect-Arduino Connector context */
static wolk_ctx_t wolk;
outbound_message_t outbound_messages[STORE_SIZE];

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, wifi_pass);

  if ( WiFi.status() != WL_CONNECTED) {
    while (WiFi.begin(ssid, wifi_pass) != WL_CONNECTED) {
      Serial.print(".");
      delay(4000);
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect_to_platform()
{
  setup_wifi();
  
  wolk_connect(&wolk);
}

void setup() {
  Serial.begin(9600);

  setup_wifi();

  wolk_init(&wolk, NULL, NULL, NULL, NULL,
            device_key, device_password, &client, hostname, portno, PROTOCOL_JSON_SINGLE, NULL, NULL);

  wolk_init_in_memory_persistence(&wolk, &outbound_messages, sizeof(outbound_messages), false);

  wolk_connect(&wolk);

  delay(1000);
  
  wolk_add_numeric_sensor_reading(&wolk, "T", 23.4, 0);

  wolk_publish(&wolk);

  delay(500);

}

void loop() {

  if (Serial.available() > 0)
  {
    wolk_disconnect(&wolk);
    Serial.println("Disconnected!");
    while (true)
    {
      delay(10000);
    };
  }
  if(wolk_process(&wolk) == W_TRUE)
  {
    reconnect_to_platform();
  }

}
