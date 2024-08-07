/* NOT VERIFIED WITH THE NEW PROTOCOL*/
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "WolkConn.h"
#include "MQTTClient.h"

const char* ssid = "wifi_ssid";
const char* wifi_pass = "wifi_password";

const char *device_key = "device_key";
const char *device_password = "device_password";
const char* hostname = "api-demo.wolkabout.com";
int portno = 8883;

/* Example variables*/
wolk_numeric_feeds_t temperature = {0};

/* WolkConnect-Arduino Connector context */
static wolk_ctx_t wolk;
outbound_message_t outbound_messages[STORE_SIZE];

extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;

WiFiClientSecure espClient;
PubSubClient client(espClient);

time_t now;
unsigned long lastMillis = 0;

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

  setup_wifi();

  // Synchronize time useing SNTP. This is necessary to verify that
  // the TLS certificates offered by the server are currently valid.
  Serial.print("Setting time using SNTP");
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));

  // Load root certificate in DER format into WiFiClientSecure object
  bool res = espClient.setCACert_P(caCert, caCertLen);
  if (!res) {
    Serial.println("Failed to load root CA certificate!");
    while (true) {
      yield();
    }
  }
  
  wolk_init(&wolk, device_key, device_password, &client, hostname, portno, PUSH, NULL, NULL);
  wolk_init_in_memory_persistence(&wolk, &outbound_messages, sizeof(outbound_messages), false);

  if(wolk_connect(&wolk))
  {
    while(reconnect_to_platform())
    {
      delay(5000);
    }
  }
  
  temperature.value = 23.4;
  if(wolk_add_numeric_feed(&wolk, "T", &temperature, 1))
  {
    Serial.println("Failed to serialise reading!");
  }

  wolk_publish(&wolk);

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
  if(wolk_process(&wolk) == W_TRUE)
  {
    reconnect_to_platform();
  }

}
