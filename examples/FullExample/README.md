# WolkConnect-Arduino
WolkAbout Arduino Connector library for connecting Arduino devices to [WolkAbout IoT platform](https://demo.wolkabout.com/#/login).

WolkConnect-Arduino is transportation layer agnostic which means it is up to the user of the library to
provide socket to WolkAbout IoT platform and configure SSL if desired.

WolkConnect-Arduino is compatible with all hardware supported by [PubSubClient](https://pubsubclient.knolleary.net/) Arduino library.
Provided example is made for Genuino MKR1000.

Supported protocol(s):
JSON single (PROTOCOL_JSON_SINGLE)

## Library usage

#### Prerequisite

	Provided example uses WiFi101 library.
  The library should be located in the Arduino /libraries folder (in case you downloaded from Git, if you installed it via Arduino Library Manager, it should already be there by default), or included in the Arduino IDE (Sketch->Include Library->Add .ZIP library).

#### Setup

Create a device on WolkAbout IoT platform by importing the provided manifest .json file located in the 'examples' folder. This manifest fits the example and demonstrates the sending of sensor readings, actuation and configuration.

Edit device information and connection parameters

```c
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
```c
WiFi.begin(ssid, password);
```

Initialize the context
```c
wolk_init(&wolk                            //ctx Context
          actuation_handler,               //function pointer to 'actuation_handler_t' implementation
          actuator_status_provider,        //function pointer to 'actuator_status_provider_t' implementation
          configuration_handler,           //function pointer to 'configuration_handler_t' implementation
          configuration_provider,          //function pointer to 'configuration_provider_t' implementation
          device_key,                      //Device key provided by WolkAbout IoT Platform upon device creation
          device_password,                 //Device password provided by WolkAbout IoT Platform device upon device creation
          &client,                         //MQQT Client
          hostname,                        //MQQT Server
          portno,                          //Port to connect to
          PROTOCOL_JSON_SINGLE,              //Protocol specified for device
          actuator_refs,                   //Array of strings containing references of actuators that device possess
          2);                              //Number of actuator references contained in actuator_references
```
Connect to server

```c
wolk_connect(&wolk);
```


#### Publishing data

As it is right now, the platform publishes data within the add sensor reading function.
Sensor types can be numeric, boolean and string and can containt more than one data parameter.
Every type has it's own function for adding value eg. numeric sensor has:

```c
wolk_add_numeric_sensor_reading(&wolk,  //Context 
                                "T",    //Sensor Reference
                                23.4,   //Sensor Value
                                0);     //UTC time of sensor value acquisition
```
and the
```c
wolk_add_multi_value_numeric_sensor_reading(&wolk,          //Context
                                            "ACL",          //Sensor reference
                                            accl_readings,  //Sensor values
                                            3,              //Number of sensor dimensions
                                            0);             //UTC time of sensor value acquisition
```

#### Alarms

Alarms are published within the add alarm function

```c
wolk_add_alarm(&wolk,   //Context
                "HH",   //Alarm Reference
                true,   //Alarm Value
                0);     //UTC time of alarm trigger
```

#### Actuation

Process receives actuation commands from the platform when wolk_process is called.
```c
wolk_process (wolk_ctx_t *ctx, uint32_t tick)
```
Actuation handler and actuator status provider functions must be implemented by the user.
Example function is included in the full example.
```c
static void actuation_handler(const char* reference, const char* value)

static actuator_status_t actuator_status_provider(const char* reference)
```

#### Configuration

Process receives configuration commands from the platform when wolk_proceess is called.
```c
wolk_process (wolk_ctx_t *ctx, uint32_t tick);
```
Configuration handler and configuration provider functions must be implemented by the user.
Example function is included in the full example.
```c
static void configuration_handler(char (*reference)[CONFIGURATION_REFERENCE_SIZE],
                                  char (*value)[CONFIGURATION_VALUE_SIZE],
                                  size_t num_configuration_items)

static size_t configuration_provider(char (*reference)[CONFIGURATION_REFERENCE_SIZE],
                                     char (*value)[CONFIGURATION_VALUE_SIZE],
                                     size_t max_num_configuration_items)
```
#### Maintaining connection to the platform
```c
wolk_process (&wolk,  //Context 
              5);     //Period at which wolk_process is called
```

### Disconnecting

Disconnecting from the platform is done by calling the
```c
wolk_disconnect(&wolk)
```
function. It sends the last will message to the platform and disconnects the MQTT client.

#### Important note

```c
wolk_process (&wolk,  //Context 
              5);     //Period at which wolk_process is called
```
Must be called regularly(less than 15 seconds between two calls) in order for the connection to be maintained.
Be wary of that while implementing loop()!
If possible, put sensor readings and other stuff to be called on timed interrupts.