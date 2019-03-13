#include <WiFi101.h>

#include "WolkConn.h"
#include "MQTTClient.h"

#define NUM_ACTUATORS 2

const char* ssid = "wifi_ssid";
const char* wifi_pass = "wifi_password";

const char *device_key = "device_key";
const char *device_password = "device_password";
const char* hostname = "api-demo.wolkabout.com";
int portno = 1883;

/* WolkConnect-Arduino Connector context */
static wolk_ctx_t wolk;
outbound_message_t outbound_messages[STORE_SIZE];

const char* actuator_refs[] = {"SL", "SW"};

static char actuator_value[NUM_ACTUATORS][READING_SIZE] = {"0", "true"};

double accl_readings[3] = {1, 3, 5};

WiFiClient espClient;
PubSubClient client(espClient);

static void actuation_handler(const char* reference, const char* value)
{
  Serial.print("Actuation handler - Reference:");
  Serial.println(reference);
  Serial.print("Value:");
  Serial.println(value);

  if (strcmp(reference, "SW") == 0) {
    strcpy(actuator_value[1], value);
  }
  else if (strcmp(reference, "SL") == 0) {
    strcpy(actuator_value[0], value);
  }
}

static actuator_status_t actuator_status_provider(const char* reference)
{
  Serial.print("Actuator status provider - Reference:");
  Serial.println(reference);

  actuator_status_t actuator_status;
  actuator_status_init(&actuator_status, "", ACTUATOR_STATE_ERROR);

  if (strcmp(reference, "SW") == 0) {
    Serial.print("Hey there SW, your new value is ");
    Serial.println(actuator_value[1]);
    actuator_status_init(&actuator_status, actuator_value[1], ACTUATOR_STATE_READY);
  }
  else if (strcmp(reference, "SL") == 0) {
    Serial.print("Hey there SL, your new value is ");
    Serial.println(actuator_value[0]);
    actuator_status_init(&actuator_status, actuator_value[0], ACTUATOR_STATE_READY);
  }

  return actuator_status;
}

static char device_configuration_references[CONFIGURATION_ITEMS_SIZE][CONFIGURATION_REFERENCE_SIZE] = {"config_1", "config_2", "config_3", "config_4"};
static char device_configuration_values[CONFIGURATION_ITEMS_SIZE][CONFIGURATION_VALUE_SIZE] = {"0", "False", "configuration_3", "configuration_4a,configuration_4b,configuration_4c"};

static void configuration_handler(char (*reference)[CONFIGURATION_REFERENCE_SIZE],
                                  char (*value)[CONFIGURATION_VALUE_SIZE],
                                  size_t num_configuration_items)
{
    for (size_t i = 0; i < num_configuration_items; ++i) {
        Serial.print("Configuration handler - Reference: ");
        Serial.print(reference[i]);
        Serial.print("Value: ");
        Serial.println(value[i]);
        
        strcpy(device_configuration_references[i], reference[i]);
        strcpy(device_configuration_values[i], value[i]);
    }
}

static size_t configuration_provider(char (*reference)[CONFIGURATION_REFERENCE_SIZE],
                                     char (*value)[CONFIGURATION_VALUE_SIZE],
                                     size_t max_num_configuration_items)
{
    WOLK_UNUSED(max_num_configuration_items);
    WOLK_ASSERT(max_num_configuration_items >= NUMBER_OF_CONFIGURATION);
    
    for (size_t i = 0; i < CONFIGURATION_ITEMS_SIZE; ++i) {
        strcpy(reference[i], device_configuration_references[i]);
        strcpy(value[i], device_configuration_values[i]);
    }
    Serial.println("Configuration provided!");
    return CONFIGURATION_ITEMS_SIZE;
}

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

  wolk_init(&wolk, actuation_handler, actuator_status_provider, configuration_handler, configuration_provider,
            device_key, device_password, &client, hostname, portno, PROTOCOL_JSON_SINGLE, actuator_refs, NUM_ACTUATORS);

  wolk_init_in_memory_persistence(&wolk, &outbound_messages, sizeof(outbound_messages), false);
  
  wolk_connect(&wolk);

  delay(1000);

  wolk_add_numeric_sensor_reading(&wolk, "T", 25.6, 0);
  wolk_add_numeric_sensor_reading(&wolk, "P", 1024, 0);
  wolk_add_numeric_sensor_reading(&wolk, "H", 52, 0);

  // Multi-value sensor reading
  wolk_add_multi_value_numeric_sensor_reading(&wolk, "ACL", accl_readings, 3, 0);

  wolk_add_alarm(&wolk, "HH", true, 0);

  wolk_publish(&wolk);
  
  wolk_publish_actuator_status(&wolk, "SW");
  wolk_publish_actuator_status(&wolk, "SL");

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

  delay(1000);
}
