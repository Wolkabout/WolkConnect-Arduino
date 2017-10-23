# WolkConnect-Arduino
WolkAbout Arduino Connector library for connecting Arduino devices to WolkAbout IoT platform.

WolkConnect-Arduino is transportation layer agnostic which means it is up to the user of the library to
open socket to WolkAbout IoT platform, configure SSL if desired, and forward read/write implementation to WolkConnect-Arduino.

WolkConnect-Arduino is compatible with all hardware supported by [PubSubClient](https://pubsubclient.knolleary.net/) Arduino library.

## Library usage
#### Setup
Edit device information and connection parameters

```
static const char* ssid = "wifi_ssid";
static const char* password = "wifi_password";

static const char *device_key = "device_key";
static const char *password_key = "password_key";

static const char* server_uri = "WOLKABOUT_IOT_PLATFORM_SERVER_URI";
static int portno = 1883;

/* Provided example uses ESP8266 WiFi module */
WiFiClient espClient;
PubSubClient client(espClient);
```

Establish WiFi connection
```
WiFi.begin(ssid, password);
```

Set desired protocol

```
wolk_set_protocol (wolk_ctx_t *ctx, protocol_type_t protocol);
```

Connect to server

```
wolk_connect (wolk_ctx_t *ctx, PubSubClient *client, const char *server, int port, const char *device_key, const char *password);
```

Set actuator references

```
wolk_set_actuator_references (wolk_ctx_t *ctx, int num_of_items, const char *item, ...);
```

When actuators are present, send initial actuator status to WolkAbout IoT platform
Depending on the actuator type following functions can be used:

```
wolk_publish_num_actuator_status (wolk_ctx_t *ctx,const char *reference,double value, actuator_status_t state, uint32_t utc_time);
```
```
wolk_publish_bool_actuator_status (wolk_ctx_t *ctx,const char *reference,bool value, actuator_status_t state, uint32_t utc_time);
```

#### Publishing data

Single readings
```
wolk_publish_single (wolk_ctx_t *ctx,const char *reference,const char *value, data_type_t type, uint32_t utc_time)
```

Aggregate readings
```
wolk_add_string_reading(wolk_ctx_t *ctx,const char *reference,const char *value, uint32_t utc_time);
wolk_add_numeric_reading(wolk_ctx_t *ctx,const char *reference,double value, uint32_t utc_time);
wolk_add_bool_reading(wolk_ctx_t *ctx,const char *reference,bool value, uint32_t utc_time);
wolk_publish (wolk_ctx_t *ctx);
```

#### Actuation

First process received commands with
```
wolk_receive (wolk_ctx_t *ctx, unsigned int timeout);
```
Then read actuation request
```
wolk_read_actuator (wolk_ctx_t *ctx, char *command, char *reference, char *value);
```

## Example
#### Setup connection
```
static const char* ssid = "wifi_ssid";
static const char* password = "wifi_password";

static const char *device_key = "device_key";
static const char *password_key = "password_key";

static const char* server_uri = "WOLKABOUT_IOT_PLATFORM_SERVER_URI";
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
#### Publish data
```
wolk_publish_single (&wolk, "reference", "23.2", DATA_TYPE_NUMERIC, 0);
```

#### Actuation
```
wolk_receive (&wolk, timeout);
wolk_read_actuator (&wolk, command, reference, value);
```

**Note:** Example application contained in 'examples' folder is compatible with ESP8266 WiFi module.