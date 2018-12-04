# WolkConnect-Arduino
WolkAbout Arduino Connector library for connecting Arduino devices to WolkAbout IoT platform.

WolkConnect-Arduino is transportation layer agnostic which means it is up to the user of the library to
open socket to WolkAbout IoT platform, configure SSL if desired, and forward read/write implementation to WolkConnect-Arduino.

WolkConnect-Arduino is compatible with all hardware supported by [PubSubClient](https://pubsubclient.knolleary.net/) Arduino library.
Provided example is made for Genuino MKR1000.
Supported protocol(s):
* JSON single

## Library usage

#### Prerequisite

	Provided example uses WiFi101 module.

#### Setup

Create a device on WolkAbout IoT platform by importing the provided manifest .json file located in the 'examples' folder. This manifest fits the example and demonstrates the sending of sensor readings, actuation and configuration.

Edit device information and connection parameters

```
static const char* ssid = "wifi_ssid";
static const char* password = "wifi_password";

static const char *device_key = "device_key";
static const char *password_key = "password_key";

static const char* hostname = "api-demo.wolkabout.com";
static int portno = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
```

Establish WiFi connection
```
WiFi.begin(ssid, password);
```

Initialize the context
```
wolk_init(wolk_ctx_t* ctx, actuation_handler_t actuation_handler, actuator_status_provider_t actuator_status_provider,
                    configuration_handler_t configuration_handler, configuration_provider_t configuration_provider,
                    const char* device_key, const char* device_password, PubSubClient *client, 
                    const char *server, int port, protocol_t protocol, const char** actuator_references,
                    uint32_t num_actuator_references)
```
Connect to server

```
wolk_connect(wolk_ctx_t *ctx);
```


#### Publishing data

As it is right now, the platform publishes data within the set sensor reading function.

#### Actuation

Process receives actuation commands from the platform when wolk_process is called.
```
wolk_process (wolk_ctx_t *ctx, unsigned int timeout)
```
Actuation handler and actuator status provider functions must be implemented by the user.
Definition is included in the full example.
```
static void actuation_handler(const char* reference, const char* value)

static actuator_status_t actuator_status_provider(const char* reference)
```

###Configuration

Process receives configuration commands from the platform when wolk_proceess is called.
```
wolk_process (wolk_ctx_t *ctx, unsigned int timeout);
```
Configuration handler and configuration provider functions must be implemented by the user.
Definition is included in the full example.
```
static void configuration_handler(char (*reference)[CONFIGURATION_REFERENCE_SIZE],
                                  char (*value)[CONFIGURATION_VALUE_SIZE],
                                  size_t num_configuration_items)

static size_t configuration_provider(char (*reference)[CONFIGURATION_REFERENCE_SIZE],
                                     char (*value)[CONFIGURATION_VALUE_SIZE],
                                     size_t max_num_configuration_items)
```
## Example
#### Setup connection
```
#include <WiFi101.h>

#include "WolkConn.h"
#include "MQTTClient.h"

const char* ssid = "wifi_ssid";
const char* wifi_pass = "wifi_password";

const char *device_key = "device_key";
const char *device_password = "device_password";
const char* hostname = "api-demo.wolkabout.com";
int portno = 1883;

/* WolkConnect-C Connector context */
static wolk_ctx_t wolk;

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

  wolk_init(&wolk, NULL, NULL, NULL, NULL,
            device_key, device_password, &client, hostname, portno, PROTOCOL_TYPE_JSON, NULL, NULL);

  wolk_connect(&wolk);

  delay (1000);
  
  wolk_add_numeric_sensor_reading(&wolk, "T", 53, 0);

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
  wolk_process (&wolk, 5);

  delay(1000);
}


```