// #include <WiFi101.h>
#include <WiFi.h>

#include "WolkConn.h"
#include "MQTTClient.h"

const char* ssid = "";
const char* wifi_pass = "";

const char *device_key = "";
const char *device_password = "";
const char* hostname = "";
int portno = 8883;

/* WolkConnect-Arduino Connector context */
static wolk_ctx_t wolk;
outbound_message_t outbound_messages[STORE_SIZE];

int counter = 0;
wolk_numeric_feeds_t feed = {0};

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

bool reconnect_to_platform()
{
  Serial.println("Reconnecting to platform...");
  setup_wifi();
  
  return wolk_connect(&wolk);
}

void setup() {
  Serial.begin(9600);
  neopixelWrite(38, 0, RGB_BRIGHTNESS, 0); // Red

  setup_wifi();

  neopixelWrite(38, 0, 0, RGB_BRIGHTNESS); // Blue

  wolk_init(&wolk, device_key, device_password, &client, hostname, portno, PUSH, NULL, NULL, NULL);

  wolk_init_in_memory_persistence(&wolk, &outbound_messages, sizeof(outbound_messages), false);

  if(wolk_connect(&wolk))
  {
    while(reconnect_to_platform())
    {
      delay(5000);
    }
  }
  randomSeed(analogRead(0));
  Serial.println("-------------------------------");
}

void loop() {
  counter += 1;
  if(counter > 5)
  {
    neopixelWrite(38, RGB_BRIGHTNESS, 0, 0); // Green
    Serial.println("Sending to platform!");
    feed.value = random(300);
    if(wolk_add_numeric_feed(&wolk, "PLCTagTemp", &feed, 1))
    {
      Serial.println("Failed to serialise reading!");
    }
    wolk_publish(&wolk);
    counter = 0;
    neopixelWrite(38, 0, 0, 0); // Black (Off)
  }
  delay(1000);

  // if (Serial.available() > 0)
  // {
  //   wolk_disconnect(&wolk);
  //   Serial.println("Disconnected!");
  //   while (true)
  //   {
  //     delay(10000);
  //   };
  // }

  // if(wolk_process(&wolk) == W_TRUE)
  // {
  //   reconnect_to_platform();
  // }

}
