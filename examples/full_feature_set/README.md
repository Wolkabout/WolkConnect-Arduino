```sh

██╗    ██╗ ██████╗ ██╗     ██╗  ██╗ ██████╗ ██████╗ ███╗   ██╗███╗   ██╗███████╗ ██████╗████████╗      
██║    ██║██╔═══██╗██║     ██║ ██╔╝██╔════╝██╔═══██╗████╗  ██║████╗  ██║██╔════╝██╔════╝╚══██╔══╝      
██║ █╗ ██║██║   ██║██║     █████╔╝ ██║     ██║   ██║██╔██╗ ██║██╔██╗ ██║█████╗  ██║        ██║         
██║███╗██║██║   ██║██║     ██╔═██╗ ██║     ██║   ██║██║╚██╗██║██║╚██╗██║██╔══╝  ██║        ██║         
╚███╔███╔╝╚██████╔╝███████╗██║  ██╗╚██████╗╚██████╔╝██║ ╚████║██║ ╚████║███████╗╚██████╗   ██║         
 ╚══╝╚══╝  ╚═════╝ ╚══════╝╚═╝  ╚═╝ ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝  ╚═══╝╚══════╝ ╚═════╝   ╚═╝         
                                                                                                       
                                                 █████╗ ██████╗ ██████╗ ██╗   ██╗██╗███╗   ██╗ ██████╗ 
                                                ██╔══██╗██╔══██╗██╔══██╗██║   ██║██║████╗  ██║██╔═══██╗
                                                ███████║██████╔╝██║  ██║██║   ██║██║██╔██╗ ██║██║   ██║
                                                ██╔══██║██╔══██╗██║  ██║██║   ██║██║██║╚██╗██║██║   ██║
                                                ██║  ██║██║  ██║██████╔╝╚██████╔╝██║██║ ╚████║╚██████╔╝
                                                ╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝  ╚═════╝ ╚═╝╚═╝  ╚═══╝ ╚═════╝ 
                                                                                                       
```
----
# WolkConnect-Arduino
WolkAbout Arduino Connector library for connecting IP based Arduino devices to [WolkAbout IoT platform](https://demo.wolkabout.com/#/login).
List of compatible hardware is driven by [PubSubClient](https://pubsubclient.knolleary.net/) Arduino library.

WolkConnect-Arduino is transportation layer agnostic which means it is up to the user of the library to
provide socket to WolkAbout IoT platform.

Provided examples are made for Genuino MKR1000. Porting to other Arduino board is done by replacing MKR1000 library with a library which fit selected board.

Supported protocol(s):
* JSON single (PROTOCOL_JSON_SINGLE)

Prerequisite
------
Following libraries are required in order to run WolkConnect-Arduino examples

  * WiFi101 library, available in [Library Manager](https://www.arduino.cc/en/Guide/Libraries).
  * Adding our library as .zip in the Arduino IDE 
    `(Sketch->Include Library->Add .ZIP library).`

Example Usage
-------------

**Initialize WolkConnect-Arduino Connector**

Create a device on WolkAbout IoT platform by importing the provided `full-example-manifest.json` file located in the `examples/full_feature_set` folder. This manifest fits `FullExample.ino` and demonstrates the sending of sensor readings, actuations and configurations.

```c
static const char *device_key = "device_key";
static const char *password_key = "password_key";

static const char* hostname = "api-demo.wolkabout.com";
static int portno = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
```

**Initialize the context**
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
          PROTOCOL_JSON_SINGLE,            //Protocol specified for device
          actuator_refs,                   //Array of strings containing references of actuators that device possess
          2);                              //Number of actuator references contained in actuator_references
wolk_connect(&wolk);
```

**Adding sensor readings:**

```c
wolk_add_numeric_sensor_reading(&wolk, "T", 23, 0);

```
and the multi value numeric function is called like so:
```c
wolk_add_multi_value_numeric_sensor_reading(&wolk,          //Context
                                            "ACL",          //Sensor reference
                                            accl_readings,  //Sensor values
                                            3,              //Number of sensor dimensions
                                            0);             //UTC time of sensor value acquisition
```

**Publishing actuator statuses:**
```c
wolk_publish_actuator_status(&wolk, "SLIDER_REF");
```
This will invoke the `actuator_status_provider` to read the actuator status, and publish actuator status.

Process receives actuation commands from the platform when `wolk_process` is called.
```c
wolk_process(wolk_ctx_t *ctx, uint32_t tick)
```
Actuation handler and actuator status provider functions must be implemented by the user.
Example function is included in the full example.
```c
static void actuation_handler(const char* reference, const char* value)

static actuator_status_t actuator_status_provider(const char* reference)
```

**Device Configuration**

Process receives configuration commands from the platform when `wolk_proceess` is called.
```c
wolk_process(wolk_ctx_t *ctx, uint32_t tick);
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

**Publishing events:**

Alarms are published with the add alarm function

```c
wolk_add_alarm(&wolk,   //Context
                "HH",   //Alarm Reference
                true,   //Alarm Value
                0);     //UTC time of alarm trigger
```

**Data publish strategy:**
Data (alarms and sensor readings) is published to the platform using
```c
wolk_publish(&wolk);
```
In between adding data and publishing, data is stored in an internal buffer (along with the unpublished
actuations and configurations, which are rare but they could be in there if you have network issues).
The buffer can store 64 values at most, so be careful. 

**Disconnecting from the platform:**

Disconnecting from the platform is done by calling:
```c
wolk_disconnect(&wolk)
```
It sends the last will message to the platform and disconnects the MQTT client.

**Cooperative scheduling:**

Fuction `wolk_process(wolk_ctx_t *ctx)` is non-blocking in order to comply with cooperative scheduling,
and it must to be called periodically.

**Important note**
```c
wolk_process(&wolk,   //Context 
              5);     //Period at which wolk_process is called
```
When implementing the `loop()` function, bear in mind that calls to `wolk_process` need to occur in intervals of less than 15 seconds apart in order to maintain the connection.
Acquiring sensor readings and other business logic should preferably be called on timed interrupts.