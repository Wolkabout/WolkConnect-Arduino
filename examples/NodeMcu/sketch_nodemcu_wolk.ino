#include "MQTTClient.h"
#include <ESP8266WiFi.h>
#include "WolkConn.h"

const char* ssid = "wifi_ssid";
const char* password = "wifi_password";
const char* mqtt_server = "api-demo.wolkabout.com";

const char *device_key = "device_key";
const char *password_key = "password_key";
int portno = 1883;
const char *numeric_slider_reference = "SL";
const char *bool_switch_reference = "SW";

char reference[32];
char command [32];
char value[64];
static wolk_ctx_t wolk;
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
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


void setup() {
  Serial.begin(9600);

  setup_wifi();

  wolk_set_protocol(&wolk, PROTOCOL_TYPE_JSON);

  wolk_connect(&wolk, &client, mqtt_server, portno, device_key, password_key);

  wolk_set_actuator_references (&wolk, 2, numeric_slider_reference, bool_switch_reference);

  wolk_publish_num_actuator_status (&wolk, numeric_slider_reference, 0, ACTUATOR_STATUS_READY, 0);

  wolk_publish_bool_actuator_status (&wolk,bool_switch_reference, true, ACTUATOR_STATUS_READY, 0);

  wolk_publish_single (&wolk, "TS", "Arduino", DATA_TYPE_STRING, 0);

  wolk_publish_single (&wolk, "TN", "30", DATA_TYPE_NUMERIC, 0);

  wolk_publish_single (&wolk, "TB", "true", DATA_TYPE_BOOLEAN, 0);

  delay (10000);

  wolk_add_bool_reading(&wolk, "TB", false, 0);

  wolk_add_string_reading(&wolk, "TS", "Miki", 0);

  wolk_add_numeric_reading(&wolk, "TN", 100, 0);

  wolk_publish (&wolk);

}

void loop() {
  memset (reference, 0, 32);
  memset (command, 0, 32);
  memset (value, 0, 64);

  wolk_receive (&wolk);
  if  (wolk_read_actuator (&wolk, command, reference, value)!= W_TRUE)
  {
    Serial.println("Wolk client - Received:");
    Serial.print("Command: ");
    Serial.println(command);
    Serial.print("Actuator reference:");
    Serial.println(reference);
    Serial.print("Value:");
    Serial.println(value);
    if (strcmp(reference, numeric_slider_reference)==0)
    {
      int num_val = atoi(value);
      wolk_publish_num_actuator_status (&wolk, numeric_slider_reference, num_val, ACTUATOR_STATUS_READY, 0);
    } else if (strcmp(reference, bool_switch_reference)==0)
    {
      if (strcmp(value,"true")==0)
      {
        wolk_publish_bool_actuator_status (&wolk,bool_switch_reference, true, ACTUATOR_STATUS_READY, 0);
      } else if (strcmp(value,"false")==0)
      {
        wolk_publish_bool_actuator_status (&wolk,bool_switch_reference, false, ACTUATOR_STATUS_READY, 0);
      }
    }
  }

  delay(1000);
}
