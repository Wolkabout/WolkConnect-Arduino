#include <WiFi101.h>

#include "WolkConn.h"
#include "MQTTClient.h"

const char* ssid = "wolkabout";
const char* wifi_pass = "Walkm3int0";

const char *device_key = "gjkowcxq0bqc7w7l";
const char *device_password = "a43f8695-0b11-418c-921e-654415750c08";
const char* hostname = "api-integration.wolksense.com";
int portno = 1883;

/* WolkConnect-C Connector context */
static wolk_ctx_t wolk;

const char* actuator_refs[] = {};

bool bool_value = true;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, wifi_pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(9600);

  setup_wifi();

  wolk_init(&wolk, NULL, NULL, device_key, device_password, &client, hostname, portno, actuator_refs, 2);

  wolk_connect(&wolk);

  delay (10000);

  wolk_add_bool_sensor_reading(&wolk, "TB", bool_value, 0);

  bool_value = !bool_value;

  wolk_add_string_sensor_reading(&wolk, "TS", "Test", 0);

  wolk_add_numeric_sensor_reading(&wolk, "TN", 100, 0);

  wolk_publish (&wolk);

}

void loop() {
  
  if (Serial.available() > 0) 
  {
    wolk_disconnect(&wolk);
    Serial.println("Disconnected!");
    while(true)
    {
      delay(10000);  
    };
  }
  wolk_receive (&wolk);

  wolk_add_bool_sensor_reading(&wolk, "TB", bool_value, 0);

  wolk_add_string_sensor_reading(&wolk, "TS", "Test", 0);

  wolk_add_numeric_sensor_reading(&wolk, "TN", random(0, 100), 0);

  wolk_publish(&wolk);

  delay(10000);
}
