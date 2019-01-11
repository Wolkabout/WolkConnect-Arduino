#include <WiFi101.h>
/*Uses the FlashStorage library for Arduino which can be found at
https://github.com/cmaglie/FlashStorage or via the Arduino Library Manager
*/
#include <FlashStorage.h>

#include "WolkConn.h"
#include "MQTTClient.h"

#define STORAGE_SIZE 32
/*Circular buffer to store outbound messages to persist*/
typedef struct{

  boolean valid;

  outbound_message_t outbound_messages[STORAGE_SIZE];

  uint32_t head;
  uint32_t tail;

  boolean empty;
  boolean full;

} Messages;

static Messages data;

const char* ssid = "wifi_ssid";
const char* wifi_pass = "wifi_password";

const char *device_key = "device_key";
const char *device_password = "device_password";
const char* hostname = "api-demo.wolkabout.com";
int portno = 1883;

/* WolkConnect-Arduino Connector context */
static wolk_ctx_t wolk;

WiFiClient espClient;
PubSubClient client(espClient);

FlashStorage(flash_store, Messages);

void _flash_store()
{
  data.valid = true;
  flash_store.write(data);
}
/*Functions for custom persistence*/
void increase_pointer(uint32_t* pointer)
{
    if ((*pointer) == (STORAGE_SIZE - 1))
    {
        (*pointer) = 0;
    }
    else
    {
        (*pointer)++;
    }
}

void _init()
{
    Serial.println("_init");
    data = flash_store.read();

    if (data.valid == false)
    {
      data.head = 0;
      data.tail = 0;

      data.empty = true;
      data.full = false;

    }
}

bool _push(outbound_message_t* outbound_message)
{
    if(data.full)
    {
        increase_pointer(&data.head);
    }

    memcpy(&data.outbound_messages[data.tail], outbound_message, sizeof(outbound_message_t));

    increase_pointer(&data.tail);
    
    data.empty = false;
    data.full = (data.tail == data.head);

    return true;
}

bool _peek(outbound_message_t* outbound_message)
{
    memcpy(outbound_message, &data.outbound_messages[data.head], sizeof(outbound_message_t));
    return true;
}

bool _pop(outbound_message_t* outbound_message)
{
    memcpy(outbound_message, &data.outbound_messages[data.head], sizeof(outbound_message_t));
    
    increase_pointer(&data.head);
    
    data.full = false;
    data.empty = (data.tail == data.head);

    return true;
}

bool _is_empty()
{
    return data.empty;
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid);

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
  
  /*Initialize the circular buffer structure*/
  _init();
  
  setup_wifi();

  wolk_init(&wolk, NULL, NULL, NULL, NULL,
            device_key, device_password, &client, hostname, portno, PROTOCOL_JSON_SINGLE, NULL, NULL);

  /*Initialize custom persistence*/
  wolk_init_custom_persistence(&wolk, _push, _peek, _pop, _is_empty);

  wolk_connect(&wolk);

  delay(1000);

  /*If there are no values in the buffer, add one*/
  if(_is_empty)
  {
    wolk_add_numeric_sensor_reading(&wolk, "T", 23.4, 0);
  }
  
  /*If data has been written to flash, publish it*/
  if(data.valid == true)
  {
    wolk_publish(&wolk);
  }

  _flash_store();

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
  if(wolk_process(&wolk, 5) == W_TRUE)
  {
    reconnect_to_platform();
  }

  delay(1000);
}
