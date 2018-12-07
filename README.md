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
wolk_init(&wolk               //ctx Context
          NULL,               //function pointer to 'actuation_handler_t' implementation
          NULL,               //function pointer to 'actuator_status_provider_t' implementation
          NULL,               //function pointer to 'configuration_handler_t' implementation
          NULL,               //function pointer to 'configuration_provider_t' implementation
          device_key,         //Device key provided by WolkAbout IoT Platform upon device creation
          device_password,    //Device password provided by WolkAbout IoT Platform device upon device creation
          &client,            //MQQT Client
          hostname,           //MQQT Server
          portno,             //Port to connect to
          PROTOCOL_JSON_SINGLE, //Protocol specified for device
          NULL,               //Array of strings containing references of actuators that device possess
          NULL);              //Number of actuator references contained in actuator_references
```
Connect to server

```c
wolk_connect(&wolk);
```


#### Publishing data
Data is added to the system using
```c
wolk_add_numeric_sensor_reading(&wolk,  //Context 
                                "T",    //Sensor Reference
                                23.4,   //Sensor Value
                                0);     //UTC time
```
Data is published to the platform using
```c
wolk_publish(&wolk);
```
In between adding data and publishing, data is stored in an internal buffer.
The buffer can store 64 values at most, so be careful. 

#### Maintaining connection to the platform
```c
wolk_process (&wolk,  //Context 
              5);     //Period at which wolk_process is called
```
#### Disconnecting

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