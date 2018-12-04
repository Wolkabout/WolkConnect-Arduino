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
static const char* ssid = "wifi_ssid";
static const char* password = "wifi_password";

static const char *device_key = "device_key";
static const char *password_key = "password_key";

static const char* server_uri = "api-demo.wolkabout.com";
static int portno = 1883;

static const char *numeric_slider_reference = "SL";
static const char *bool_switch_reference = "SW";

static char reference[32];
static char command [32];
static char value[64];
static static wolk_ctx_t wolk;
static WiFiClient espClient;
static PubSubClient client(espClient);

/* Provided example uses ESP8266 WiFi module */
static WiFiClient espClient;
static PubSubClient client(espClient);

void setup() {
	delay(10);
	WiFi.begin(ssid, password);
	
	while (WiFi.status() != WL_CONNECTED) {
	    delay(100);
	}
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
	
	wolk_add_string_reading(&wolk, "TS", "Example_String_Reading", 0);
	
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

```