/*
 * ESP8266 MQTT Wifi Client to Serial Bridge with NTP
 * Author: rkubera https://github.com/rkubera/
 * License: MIT
 */

#define MQTTBRIDGE_VERSION_MAJOR 1
#define MQTTBRIDGE_VERSION_MINOR 0

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <String.h>

#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#include <coredecls.h>                  // settimeofday_cb()

//MQTT
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);
char mqtt_server[41] = "";
char mqtt_port[7] = "1883";
char mqtt_user[33] = "";
char mqtt_pass[33] = "";
char wifi_hostname[33] = "mqtt-client";

String payloadsBuffer = "";
String mqtt_allSubscriptions = "";

//EEPROM
#include <EEPROM.h>
char mqtt_versionAddress = 0;
int tempEepromAddress = mqtt_versionAddress+2;
int tempSelectorAddress = tempEepromAddress+1;
char mqtt_serverAddress = tempSelectorAddress+1;
char mqtt_portAddress = mqtt_serverAddress+41;
char mqtt_userAddress = mqtt_portAddress+7;
char mqtt_passAddress = mqtt_userAddress+33;
char wifi_hostnameAddress = mqtt_passAddress+33;

//Wifi
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include "WiFiManager.h"        //https://github.com/tzapu/WiFiManager

WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 32);
WiFiManagerParameter custom_mqtt_pass("pass", "mqtt password", mqtt_pass, 32);
WiFiManagerParameter custom_wifi_hostname("hostname", "hostname", wifi_hostname, 32);
  
WiFiManager wifiManager;
String ssid = "mqtt-clnt-"+String(ESP.getChipId());
String password;
bool wificonnected = false;

void saveConfigCallback () {
  EEPROM.write(mqtt_versionAddress, MQTTBRIDGE_VERSION_MAJOR);
  EEPROM.write(mqtt_versionAddress+1, MQTTBRIDGE_VERSION_MINOR);
    
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_pass, custom_mqtt_pass.getValue());
  strcpy(wifi_hostname, custom_wifi_hostname.getValue());

  saveToEeprom (mqtt_serverAddress, mqtt_server, 41);
  saveToEeprom (mqtt_portAddress, mqtt_port, 7);
  saveToEeprom (mqtt_userAddress, mqtt_user, 33);
  saveToEeprom (mqtt_passAddress, mqtt_pass, 33);
  saveToEeprom (wifi_hostnameAddress, wifi_hostname, 33);
}

//NTP
#define TZ              0       // (utc+) TZ in hours
#define DST_MN          0      // use 60mn for summer time in some countries
#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)
timeval cbtime;      // time set in callback
bool cbtime_set = false;

//CRC32
bool crc32Enabled = true;

//BUFFER
#define bufferSize 1024
uint8_t myBuffer[bufferSize];
int bufIdx=0;
unsigned long buffmillis = 0;
int intervalMillis = 200;

//MQTT payload
long lastMsg = 0;
char msg[bufferSize];
long int value = 0;
bool mqtt_was_connected = false;

void time_is_set_cb(void) {
  gettimeofday(&cbtime, NULL);
  cbtime_set = true;
  if (ssid!="" && WiFi.status() == WL_CONNECTED) {
    cbtime_set = true;
  }
  else {
    cbtime_set = false;
  }
}

void mqtt_cb(char* topic, byte* payload, unsigned int length) {
  if (crc32Enabled) {
    CRC32_reset();
    for (size_t i = 0; i < strlen(topic); i++) {
      CRC32_update(topic[i]);
    }
    
    CRC32_update(' ');
    
    for (size_t i = 0; i < length; i++) {
      CRC32_update(payload[i]);
    }
    uint32_t checksum = CRC32_finalize();

    payloadsBuffer+=String(checksum)+" ";
    
    //Serial.print(checksum);
    //Serial.print(" ");
  }
  payloadsBuffer+=String(topic)+" ";
  for (size_t i = 0; i < length; i++) {
    payloadsBuffer+=(char)payload[i];
  }
  payloadsBuffer+="\n";
  
  if ((char)payload[0] == '1') {
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  } else {
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

void reSubscribe() {
  int start = 0;
  int lineIdx;
  String sub;
  do {
    lineIdx = mqtt_allSubscriptions.indexOf('\n', start);
    if (lineIdx>-1) {
      sub = mqtt_allSubscriptions.substring(start, lineIdx);
      start = lineIdx+1;
      sub.trim();
    }
    else {
      sub = "";
    }
    if (sub.length()>0) {
      client.subscribe(sub.c_str());
    }
  }
  while (lineIdx>-1);
}

void resubcribe() {
  int start = 0;
  int lineIdx;
  String sub;
  do {
    lineIdx = mqtt_allSubscriptions.indexOf('\n', start);
    if (lineIdx>-1) {
      sub = mqtt_allSubscriptions.substring(start, lineIdx);
      start = lineIdx+1;
      sub.trim();
    }
    else {
      sub = "";
    }
    if (sub.length()>0) {
      client.subscribe(sub.c_str());
    }
  }
  while (lineIdx>-1);
}

bool reconnect() {
    mqtt_was_connected = false;
    // Attempt to connect
    if (mqtt_user!="") {
      int i_mqtt_port ;
      sscanf(mqtt_port, "%d", &i_mqtt_port);
      client.setServer(mqtt_server, i_mqtt_port);
      delay(300);
      if (client.connect(wifi_hostname, mqtt_user, mqtt_pass)) {
        sendCommand("mqtt connected");
        resubcribe();
        mqtt_was_connected = true;
        return true;
      }
    }
    else {
      if (client.connect(wifi_hostname)) {
        sendCommand("mqtt connected");
        resubcribe();
        mqtt_was_connected = true;
        client.setCallback(mqtt_cb);
        delay(100);
        return true;
      }
    }
    return false;
}

void saveToEeprom (int addr, char* value, int size) {
  for (int i=0; i<size; i++) {
    EEPROM.write(addr+i, value[i]); 
  }
  EEPROM.commit();
}

void loadFromEeprom (int addr, char* value, int size) {
  for (int i=0; i<size; i++) {
    value[i] = EEPROM.read(addr+i);
  }
}

void mqttloop() {
  commandLoop();
  client.loop();
  yield();
}

void mydelay(int millisDelay) {
  unsigned long mytimer = millis();
  while (abs(millis()-mytimer)<millisDelay) {
    mqttloop();
  }
}

void setup() {

  strcpy(wifi_hostname,ssid.c_str());
  //ESP.eraseConfig();
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

  //Serial
  Serial.begin(9600);
  Serial.println();
  sendCommand("ready");

  //EEPROM
  EEPROM.begin(512);
  if (EEPROM.read(mqtt_versionAddress)==MQTTBRIDGE_VERSION_MAJOR && EEPROM.read(mqtt_versionAddress+1)==MQTTBRIDGE_VERSION_MINOR) {
    sendCommand("loadConfig");
    loadFromEeprom (mqtt_serverAddress, mqtt_server, 41);
    loadFromEeprom (mqtt_portAddress, mqtt_port, 7);
    loadFromEeprom (mqtt_userAddress, mqtt_user, 33);
    loadFromEeprom (mqtt_passAddress, mqtt_pass, 33);
    loadFromEeprom (wifi_hostnameAddress, wifi_hostname, 33);
   }
   else {
    saveConfigCallback ();
   }
  
  //WIFI
  custom_mqtt_server.setValue(mqtt_server,40);
  custom_mqtt_port.setValue(mqtt_port,6);
  custom_mqtt_user.setValue(mqtt_user,32);
  custom_mqtt_pass.setValue(mqtt_pass,32);
  custom_wifi_hostname.setValue(wifi_hostname,32);
  
  WiFi.hostname(wifi_hostname);
  client.setCallback(mqtt_cb);

  //NTP
  settimeofday_cb(time_is_set_cb);
  configTime(TZ_SEC, DST_SEC, "pool.ntp.org");  

  wifiManager.setDebugOutput(false);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setConnectTimeout(5);
  wifiManager.setPortalLoopCallback(mqttloop);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);
  wifiManager.addParameter(&custom_wifi_hostname);
}

void loop() {
  wificonnected = false;
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setSaveConnectTimeout(3);
  if (WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_CONNECTION_LOST || WiFi.status() == WL_DISCONNECTED) {
    if (!wifiManager.autoConnect(ssid.c_str())) {
      unsigned long mytimer = millis();
      while (abs(millis()-mytimer)<3000) {
        mqttloop();
      }
    }
    if (WiFi.status() == WL_CONNECTED) {
      ssid = wifiManager.getWiFiSSID(false);
      sendCommand("wifi connected");
    }
  }
  else {
    wificonnected = true;
    mqttloop();
  }
}
