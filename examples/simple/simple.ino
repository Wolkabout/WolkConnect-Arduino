/*
 * Copyright 2024 WolkAbout Technology s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <WiFi.h>

#include "WolkConn.h"
#include "MQTTClient.h"

const char* ssid = "";
const char* wifi_pass = "";

const char *device_key = "device_key";
const char *device_password = "password_key";
const char* hostname = "wolkabout-platform-instance-url";
int portno = 8883;

/* Example variables*/
int loop_counter = 0;
wolk_numeric_feeds_t temperature = {0};

/* WolkConnect-Arduino Connector context */
static wolk_ctx_t wolk;
outbound_message_t outbound_messages[STORE_SIZE];

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
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

  wolk_init(&wolk, device_key, device_password, &client, hostname, portno, PUSH, NULL, NULL);
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
  loop_counter ++;
  if(loop_counter > 5)
  {
    neopixelWrite(38, RGB_BRIGHTNESS, 0, 0); // Green
    Serial.println("Sending to platform!");
    temperature.value = random(300);
    if(wolk_add_numeric_feed(&wolk, "T", &temperature, 1))
    {
      Serial.println("Failed to serialise reading!");
    }
    wolk_publish(&wolk);
    loop_counter = 0;
    neopixelWrite(38, 0, 0, 0); // Black (Off)
  }
  delay(1000);

  if (Serial.available() > 0)
  {
    wolk_disconnect(&wolk);
    Serial.println("Disconnected!");
    neopixelWrite(38, 0, RGB_BRIGHTNESS, 0); // Red
    while (true)
    {
      delay(10000);
    };
  }

  if(wolk_process(&wolk) == W_TRUE)
  {
    reconnect_to_platform();
  }
} // void loop()
