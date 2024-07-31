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
# IMPORTANT
**Compatible from WolkAbout Platform 22.GA version.**

----
# WolkConnect-Arduino
WolkAbout Arduino Connector library for connecting IP based Arduino devices to WolkAbout IoT platform instance.
List of compatible hardware is driven by [PubSubClient](https://pubsubclient.knolleary.net/) Arduino library.

WolkConnect-Arduino is transportation layer agnostic which means it is up to the user of the library to
provide socket to WolkAbout IoT platform.

Provided examples are made for ESP32 boards.

Supported protocol(s):
* WOLKABOUT PROTOCOL

Prerequisite
------
Following libraries are required in order to run WolkConnect-Arduino examples

  * WiFi arduino library for supporting ESP32 chipset
  * WiFi101 library, available in [Library Manager](https://www.arduino.cc/en/Guide/Libraries).
  * WolkConnect library, available in [Library Manager](https://www.arduino.cc/en/Guide/Libraries).
  Alternatively you can import it as a .zip library in the Arduino IDE.

Example Usage
-------------

**Initialize WolkConnect-Arduino Connector**

Create a device on WolkAbout IoT Platform by using the *Simple example* device type that is available on the platform. 
This device type fits `simple.ino` and demonstrates the periodic sending of a temperature sensor reading.

```c
static const char *device_key = "device_key";
static const char *password_key = "password_key";

static const char* hostname = "wolkabout-platform-instance-url";
static int portno = 8883;

WiFiClient espClient;
PubSubClient client(espClient);
```

**Initialize the context**
```c
wolk_init(&wolk                 //ctx Context
          device_key,           //Device key provided by WolkAbout IoT Platform upon device creation
          device_password,      //Device password provided by WolkAbout IoT Platform device upon device creation
          &client,              //MQQT Client
          hostname,             //MQQT Server
          portno,               //Port to connect to
          PUSH,                 //Device outbound mode - see outbound_mode_t
          NULL,                 //Feeds handler        - see feed_handler_t
          NULL);                //Error handler        - see error_handler_t
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
wolk_numeric_feeds_t feed = {0};
feed.value = 23;
wolk_add_numeric_feed(&wolk, "T", &feed, 1)
```
**Data publish strategy:**

In between adding feeds and publishing, feeds are stored in an user-defined buffer passed via persistence functions.
The buffer stores an array of outbound_message_t which is 320 bytes in length.

Stored feeds are pushed to WolkAbout IoT platform on demand by calling:
```c
wolk_publish(&wolk);
```

**Cooperative scheduling:**

Fuction `wolk_process(wolk_ctx_t *ctx)` is non-blocking in order to comply with cooperative scheduling,
and it must to be called periodically.
```c
wolk_process(&wolk);
```
**Getting the epoch:** TODO

Epoch time is stored in the wolk_ctx_t structure field named epoch_time and it's value is updated on every pong received.
Optionally, you can update it by calling wolk_update_epoch(). Processing must be periodically called as this information is 
received from the platform.
```c
wolk_update_epoch(&wolk);
/*call processing until pong is received*/
while (!(wolk.pong_received)) {
    wolk_process(&wolk, 5);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
  }
```

**Disconnecting from the platform:**
```c
wolk_disconnect(&wolk)
```

# Important note

When implementing the `loop()` function, bear in mind that calls to `wolk_process` need to occur in intervals of less than a minute apart in order to maintain the connection.
Acquiring sensor readings and other business logic should preferably be called on timed interrupts.

# Known issues

When connecting MKR1000 or Node MCU to a busy or unstable WiFi network, MQTT client might lose connection often. MQTT client state will return -3 error code.
The example handles this situation by reconnecting to the WiFi and the platform. 
If the broker can't be reached MQTT Client state will return -4 error code.
Here is a list of possible MQTT state codes:

|Code| Error             | 
|----|-------------------|
|-4  |connection timeout |
|-3  |connection lost    |
|-2  |connect failed     |
|-1  |disconected        |
| 0  |connected          |
| 1  |bad protocol       |
| 2  |bad client id      |
| 3  |unavailable        |
| 4  |bad credentials    |
| 5  |unauthorized       |
