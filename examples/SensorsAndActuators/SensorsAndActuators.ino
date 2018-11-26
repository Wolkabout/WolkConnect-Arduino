#include <WiFi101.h>

#include "WolkConn.h"
#include "MQTTClient.h"

const char* ssid = "wolkabout";
const char* wifi_pass = "Walkm3int0";

const char *device_key = "8o78cqnpfg70hu5v";
const char *device_password = "9363b99d-0165-461a-b521-c1e39a862061";
const char* hostname = "api-demo.wolkabout.com";
int portno = 1883;

/* WolkConnect-C Connector context */
static wolk_ctx_t wolk;

const char* actuator_refs[] = {"SL", "SW"};

static char actuator_value[READING_SIZE] = {"0"};

WiFiClient espClient;
PubSubClient client(espClient);

static void actuation_handler(const char* reference, const char* value)
{
    Serial.print("Actuation handler - Reference:");
    Serial.println(reference);
    Serial.print("Value:");
    Serial.println(value);

    strcpy(actuator_value, value);
}

static actuator_status_t actuator_status_provider(const char* reference)
{
    Serial.print("Actuator status provider - Reference:");
    Serial.println(reference);

    actuator_status_t actuator_status;
    actuator_status_init(&actuator_status, "", ACTUATOR_STATE_ERROR);

    if (strcmp(reference, "SW") == 0) {
        Serial.print("Hey there SW, your new value is ");
        Serial.println(actuator_value);
        actuator_status_init(&actuator_status, actuator_value, ACTUATOR_STATE_READY);
    }
    else if (strcmp(reference, "SL") == 0) {
        Serial.print("Hey there SL, your new value is ");
        Serial.println(actuator_value);
        actuator_status_init(&actuator_status, actuator_value, ACTUATOR_STATE_READY);
    }

    return actuator_status;
}

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

  wolk_init(&wolk, actuation_handler, actuator_status_provider, device_key, device_password, &client, hostname, portno, actuator_refs, 2);

  wolk_connect(&wolk);

  delay (1000);

  wolk_add_bool_sensor_reading(&wolk, "TB", false, 0);

  wolk_add_string_sensor_reading(&wolk, "TS", "Miki", 0);

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
  wolk_process (&wolk);

  delay(1000);
}
