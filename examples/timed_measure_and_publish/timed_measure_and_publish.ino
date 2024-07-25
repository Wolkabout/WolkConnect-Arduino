#include <WiFi101.h>
#include <RTCZero.h>

#include "WolkConn.h"
#include "MQTTClient.h"

const char* ssid = "ssid";
const char* wifi_pass = "wifi_pass";

const char *device_key = "device_key";
const char *device_password = "device_password";
const char* hostname = "api-demo.wolkabout.com";
int portno = 1883;

/* WolkConnect-Arduino Connector context */
static wolk_ctx_t wolk;
outbound_message_t outbound_messages[STORE_SIZE];

WiFiClient espClient;
PubSubClient client(espClient);

RTCZero rtc;
/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 17;

/* Change these values to set the current initial date */
const byte day = 17;
const byte month = 11;
const byte year = 15;
bool read;
/*Read sensor every 15 seconds. If you change this parameter
make sure that it's <60*/
const byte readEvery = 15;
bool publish;
/*Publish every minute. If you change this parameter
make sure that it's <60*/
const byte publishEvery = 1;
byte publishMinutes = (minutes + publishEvery) % 60;

void setup_wifi() {

  delay(10);
  /* We start by connecting to a WiFi network*/
  WiFi.begin(ssid);

  if ( WiFi.status() != WL_CONNECTED) {
    while (WiFi.begin(ssid, wifi_pass) != WL_CONNECTED) {
      delay(300);
    }
  }
}

void reconnect_to_platform()
{
  setup_wifi();
  
  wolk_connect(&wolk);
}

void setup() {
  Serial.begin(9600);

  setup_wifi();

  wolk_init(&wolk, NULL, NULL, NULL, NULL,
            device_key, device_password, &client, hostname, portno, NULL, NULL);

  wolk_init_in_memory_persistence(&wolk, &outbound_messages, sizeof(outbound_messages), false);

  delay(1000);

  rtc.begin();

  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  rtc.setAlarmTime(17, 00, 10);
  rtc.enableAlarm(rtc.MATCH_SS);

  rtc.attachInterrupt(alarmMatch);

  rtc.standbyMode();

}

void loop() {

  if(read)
  {
    read = false;
    
    wolk_add_numeric_sensor_reading(&wolk, "T", random(-20, 80), 0);
    
    /*set new alarm in 30 seconds*/
    int alarmSec = (rtc.getSeconds() + readEvery) % 60;
    rtc.setAlarmSeconds(alarmSec);
    delay(100);
  }
  
  if(publish)
  {
    publish = false;
    wolk_connect(&wolk);
    wolk_publish(&wolk);
    /*set new alarm in 15 minutes*/
    publishMinutes = (rtc.getMinutes() + publishEvery) % 60;
    delay(100);
    wolk_disconnect(&wolk);
    delay(100);
  }
  rtc.standbyMode();

}

void alarmMatch()
{
  read = true;
  if(publishMinutes == rtc.getMinutes())
  {
    publish = true;
  }
}
