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
```

**Initialize in-memory persistence**
```c
wolk_init_in_memory_persistence(&wolk,                       // Context
                                persistence_storage,         // Address to start of the memory which will be used by persistence mechanism 
                                sizeof(persistence_storage), // Size of memory in bytes
                                true);                       // If storage is full overwrite oldest item when pushing
```

**Establishing connection with WolkAbout IoT platform**

```c
wolk_connect(&wolk);
```

**Adding sensor readings:**

```c
wolk_add_numeric_sensor_reading(&wolk, "T", 25.6, 0);

// Multi-value sensor reading
wolk_add_multi_value_numeric_sensor_reading(&wolk, "ACL", accl_readings, 3, 0);
```

**Adding events:**
```c
wolk_add_alarm(&wolk, "HH", true, 0);
```

**Data publish strategy:**

Data (alarms and sensor readings) is published to the platform using
```c
wolk_publish(&wolk);
```
In between adding data and publishing, data is stored in an user-defined buffer passed via persistence functions (along with the unpublished
actuations and configurations, which are rare but they could be in there if you have network issues).
The buffer stores an array of outbound_message_t which is 320 bytes in length. 

**Publishing actuator statuses:**
```c
wolk_publish_actuator_status(&wolk, "SW");
```
This will invoke the `actuator_status_provider` to read the actuator status, and publish actuator status. 

**Publishing configuration**
```c
wolk_publish_configuration(&wolk);
```
This will call the `ConfigurationProvider` to read the current configuration and publish it to the platform

**Cooperative scheduling:**

Fuction `wolk_process(wolk_ctx_t *ctx)` is non-blocking in order to comply with cooperative scheduling,
and it must to be called periodically.
```c
wolk_process(&wolk, 5);
```

**Disconnecting from the platform:**
```c
wolk_disconnect(&wolk)
```

# Important note

When implementing the `loop()` function, bear in mind that calls to `wolk_process` need to occur in intervals of less than 15 seconds apart in order to maintain the connection.
Acquiring sensor readings and other business logic should preferably be called on timed interrupts.

# Known issues

When connecting MKR1000 or Node MCU to a busy or unstable WiFi network, MQTT client might lose connection often. 
The example handles this situation by reconnecting to the WiFi and the platform. 
MQTT client state will return -3 error code.
If the broker can't be reached MQTT Client state will return -4 error code.