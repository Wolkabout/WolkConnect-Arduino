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
  * [FlashStorage](https://github.com/cmaglie/FlashStorage) library available in  [Library Manager](https://www.arduino.cc/en/Guide/Libraries).
  * Adding our library as .zip in the Arduino IDE 
    `(Sketch->Include Library->Add .ZIP library).`

Example Usage
-------------

**Initialize WolkConnect-Arduino Connector**

Create a device on WolkAbout IoT platform by importing the provided `simple-example-manifest.json` file located in the `examples/simple` folder. This manifest fits `SimpleExample.ino` and demonstrates the sending of a temperature sensor reading.

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
wolk_init(&wolk                 //ctx Context
          NULL,                 //function pointer to 'actuation_handler_t' implementation
          NULL,                 //function pointer to 'actuator_status_provider_t' implementation
          NULL,                 //function pointer to 'configuration_handler_t' implementation
          NULL,                 //function pointer to 'configuration_provider_t' implementation
          device_key,           //Device key provided by WolkAbout IoT Platform upon device creation
          device_password,      //Device password provided by WolkAbout IoT Platform device upon device creation
          &client,              //MQQT Client
          hostname,             //MQQT Server
          portno,               //Port to connect to
          PROTOCOL_JSON_SINGLE, //Protocol specified for device
          NULL,                 //Array of strings containing references of actuators that device possess
          NULL);                //Number of actuator references contained in actuator_references
```
**Initialize custom persistence**

See persistence.h for signatures of methods to be implemented, and implementation contract.
```c
wolk_init_custom_persistence(&wolk,      //Context
                              push,       //Function pointer to 'push' implemenation
                              peek,       //Function pointer to 'pop' implementation
                              pop,        //Function pointer to 'peek' implementation
                              is_empty);  //Function pointer to 'is empty' implementation
```

**Establishing connection with WolkAbout IoT platform**

```c
wolk_connect(&wolk);
```

**Adding sensor readings:**
```c
wolk_add_numeric_sensor_reading(&wolk, "T", 23.4, 0);
```
**Data publish strategy:**

In between adding data and publishing, data is stored in an user-defined buffer passed via persistence functions (along with the unpublished
actuations and configurations, which are rare but they could be in there if you have network issues).
The buffer stores an array of outbound_message_t which is 320 bytes in length.

Stored sensor readings are pushed to WolkAbout IoT platform on demand by calling:
```c
wolk_publish(&wolk);
```

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

**Additional functionality**

WolkConnect-Arduino library has integrated additional features which can perform full WolkAbout IoT platform potential. Read more about full feature set example [HERE](https://github.com/Wolkabout/WolkConnect-Arduino/tree/master/examples/full_feature_set).

# Important note

When implementing the `loop()` function, bear in mind that calls to `wolk_process` need to occur in intervals of less than 15 seconds apart in order to maintain the connection.
Acquiring sensor readings and other business logic should preferably be called on timed interrupts.
