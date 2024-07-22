// #include <WiFi101.h>
#include <WiFi.h>

#include "WolkConn.h"
#include "MQTTClient.h"

// const char* ssid = "Cpku";
// const char* wifi_pass = "Welcome26";

const char* ssid = "guest";
const char* wifi_pass = "g3tm3int0";

const char *device_key = "wca";
const char *device_password = "OLCHCJKC8M";
const char* hostname = "showcase.wolkabout.com";
int portno = 2883; //unsecure

/* WolkConnect-Arduino Connector context */
static wolk_ctx_t wolk;
outbound_message_t outbound_messages[STORE_SIZE];

int counter = 0;

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

void reconnect_to_platform()
{
  setup_wifi();
  
  wolk_connect(&wolk);
}

void setup() {
  Serial.begin(9600);
    neopixelWrite(38, 0, RGB_BRIGHTNESS, 0); // Red

  setup_wifi();

  neopixelWrite(38, 0, 0, RGB_BRIGHTNESS); // Blue

  wolk_init(&wolk, NULL, NULL, NULL, NULL,
            device_key, device_password, &client, hostname, portno, PROTOCOL_WOLKABOUT, NULL, NULL);

  wolk_init_in_memory_persistence(&wolk, &outbound_messages, sizeof(outbound_messages), false);

  wolk_connect(&wolk);

  delay(1000);
}

void loop() {
  counter += 1;
  if(counter > 5)
  {
    neopixelWrite(38, RGB_BRIGHTNESS, 0, 0); // Green
    Serial.println("Sending to platform!");
    wolk_add_numeric_sensor_reading(&wolk, "PLCTagTemp", counter, 0);
    wolk_publish(&wolk);
    counter = 0;
    neopixelWrite(38, 0, 0, 0); // Black (Off)
  }
  delay(1000);

  if (Serial.available() > 0)
  {
    wolk_disconnect(&wolk);
    Serial.println("Disconnected!");
    while (true)
    {
      delay(10000);
    };
  }
  if(wolk_process(&wolk) == W_TRUE)
  {
    reconnect_to_platform();
  }

}
