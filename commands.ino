/*
 * ESP8266 MQTT Wifi Client to Serial Bridge with NTP
 * Author: rkubera https://github.com/rkubera/
 * License: MIT
 */
 
#include <String.h>

void commandLoop() {
 if (!client.connected() && mqtt_server!="") {
    reconnect();
  }
  if (getSerialBuffer((char*)myBuffer, bufIdx)) {
    parseLines((char*)myBuffer);
  }
  parseBuffer();
}

void sendCommand (String myCommand) {
  if (crc32Enabled) {
    CRC32_reset();
    CRC32_update('[');
    for (size_t i = 0; i < myCommand.length(); i++) {
      CRC32_update(myCommand[i]);
    }
    CRC32_update(']');
    uint32_t checksum = CRC32_finalize();
    
    Serial.print(checksum);
    Serial.print(" ");
  }
  Serial.print("[");
  Serial.print(myCommand);
  Serial.println("]");
}

void getCommand(String payload) {
  if (payload=="timestamp") {
    uint32_t mytimestamp = 0;
    mytimestamp = time(nullptr);
    String command = "timestamp ";
    if (cbtime_set==true) {
        mytimestamp = time(nullptr);
        if (mytimestamp<1000000) {
          mytimestamp = 0;
        }
    }
    command = command+(String) mytimestamp;
    sendCommand(command);
  }
  else if (payload=="echo") {
    sendCommand("echo");
  }
  else if (payload=="ip") {
    IPAddress ip =  WiFi.localIP();
    String command = 
      String(ip[0]) + "." +\
      String(ip[1]) + "." +\
      String(ip[2]) + "." +\
      String(ip[3]); 
    sendCommand(command);
  }
  else if (payload=="wifistatus") {
    if (WiFi.status() == WL_CONNECTED) {
      sendCommand("wifi connected");
      wificonnected = true;
    }
    else {
      sendCommand("wifi not connected");
      wificonnected = false;
    }
  }
  else if (payload=="mqttstatus") {
    if (client.connected()) {
      sendCommand("mqtt connected");
    }
    else {
      sendCommand("mqtt not connected");
    }
  }
  else if (payload=="crc32status") {
    if (crc32Enabled==true) {
      sendCommand("crc32 on");
    }
    else {
      sendCommand("crc32 off");
    }
  }
  else if (payload=="ssid") {
    sendCommand("ssid "+ssid);
  } 
  else if (payload=="mqttserver") {
    sendCommand("mqttserver "+String(mqtt_server));
  }
  else if (payload=="mqttport") {
    sendCommand("mqttport "+(String)mqtt_port);
  }
  else if (payload=="mqttuser") {
    sendCommand("mqttuser "+String(mqtt_user));
  }
  else if (payload=="hostname") {
    sendCommand("hostname "+String(wifi_hostname));
  }
  else sendCommand("error");
}

void publishCommand (String payload) {
  if (!client.connected()) {
    sendCommand("mqtt not connected");
    return;
  }
  int tmpIdx;
  tmpIdx = payload.indexOf(' ');
  if (tmpIdx>-1) {
    client.publish(payload.substring(0, tmpIdx).c_str(),payload.substring(tmpIdx+1).c_str());
    sendCommand("published");
  }
  else {
    sendCommand("wrong publish command");
  }
}

void publishretainedCommand (String payload) {
  if (!client.connected()) {
    sendCommand("mqtt not connected");
    return;
  }
  int tmpIdx;
  tmpIdx = payload.indexOf(' ');
  if (tmpIdx>-1) {
    client.publish(payload.substring(0, tmpIdx).c_str(),payload.substring(tmpIdx+1).c_str(), true);
    sendCommand("published");
  }
  else {
    sendCommand("wrong publish command");
  }
}

void crc32Command(String payload) {
  if (payload=="on" || payload=="ON") {
    crc32Enabled = true;
    sendCommand("crc32 on");
  }
  else {
    crc32Enabled = false;
    sendCommand("crc32 off");
  }
}

void connectCommand(String payload) {
  int tmpIdx;
  tmpIdx = payload.indexOf(':');

  String new_ssid;
  String new_password;
  if (tmpIdx>-1) {
    //AP ssid and password
    new_ssid = payload.substring(0, tmpIdx);
    new_password = payload.substring(tmpIdx+1);
  }
  else {
    //open AP, only ssid
    new_ssid = payload;
  }
  //int res =  wifiManager.connectWifi(ssid, password);
  wifiManager.setConfigPortalTimeout(1);

  struct station_config station_cfg;
  
  if (new_ssid  == ssid && new_password == password) {
    WiFi.persistent(false);
  }
  else {
    WiFi.persistent(true);
  }
  int res = WiFi.begin(new_ssid.c_str(), new_password.c_str(),0,NULL,true);
  ssid = new_ssid;
  WiFi.persistent(false);
  sendCommand("connecting to wifi");
}

void wifiHostnameCommand(String payload) {
  if (strcmp(payload.c_str(),wifi_hostname)) {
      strcpy(wifi_hostname, payload.c_str());
      saveToEeprom (wifi_hostnameAddress, wifi_hostname, 33);
      custom_wifi_hostname.setValue(wifi_hostname,32);
    }
    wifi_station_set_hostname(wifi_hostname);
    WiFi.hostname(wifi_hostname);
    sendCommand("hostname changed");
}


void mqttUserPassCommand(String payload) {
  int tmpIdx;
  tmpIdx = payload.indexOf(':');
  bool changed = false;
  if (tmpIdx>-1) {
    String new_mqtt_user = payload.substring(0, tmpIdx);
    String new_mqtt_pass = payload.substring(tmpIdx+1);

    if (strcmp(new_mqtt_user.c_str(),mqtt_user)) {
      strcpy(mqtt_user, new_mqtt_user.c_str());
      saveToEeprom (mqtt_userAddress, mqtt_user, 33);
      custom_mqtt_user.setValue(mqtt_user,32);
      changed = true;
    }

    if (strcmp(new_mqtt_pass.c_str(),mqtt_pass)) {
      strcpy(mqtt_pass, new_mqtt_pass.c_str());
      saveToEeprom (mqtt_passAddress, mqtt_pass, 33);
      custom_mqtt_pass.setValue(mqtt_pass,32);
      changed = true;
    }
    
  }
  else {
    //user only
    if (strcmp(payload.c_str(),mqtt_user)) {
      strcpy(mqtt_user, payload.c_str());
      strcpy(mqtt_pass, "");
      saveToEeprom (mqtt_userAddress, mqtt_user, 33);
      saveToEeprom (mqtt_passAddress, mqtt_pass, 33);
      custom_mqtt_user.setValue(mqtt_user,32);
      custom_mqtt_pass.setValue(mqtt_pass,32);
      changed = true;
    }
    //mqtt_user = payload.c_str();
  }

  if (changed==true) {
    mqtt_allSubscriptions = "";
    client.disconnect();
  }
  sendCommand("mqtt user and pass set");
}

void mqttServerCommand(String payload) {
  int tmpIdx;
  tmpIdx = payload.indexOf(':');
  String new_mqtt_server;
  String new_mqtt_port;
  if (tmpIdx>-1) {
    new_mqtt_server = payload.substring(0, tmpIdx);
    new_mqtt_port = payload.substring(tmpIdx+1);
  }
  else {
    new_mqtt_server = payload;
    new_mqtt_port = "1883";
  }

  bool changed = false;
  if (strcmp(new_mqtt_server.c_str(),mqtt_server)) {
    strcpy(mqtt_server, new_mqtt_server.c_str());
    saveToEeprom (mqtt_serverAddress, mqtt_server, 41);
    custom_mqtt_server.setValue(mqtt_server,40);
    changed = true;
  }

  if (strcmp(new_mqtt_port.c_str(),mqtt_port)) {
    strcpy(mqtt_port, new_mqtt_port.c_str());
    saveToEeprom (mqtt_portAddress, mqtt_port, 7);
    custom_mqtt_port.setValue(mqtt_port,6);
    changed = true;
  }

  if (changed==true) {
    client.disconnect();
    mqtt_allSubscriptions = "";
    mqtt_was_connected=false;
  }
  sendCommand("connecting to mqtt server");
}

void subscribeCommand (String subscription) {
  int start = 0;
  int lineIdx;
  String sub;
  bool found = false;
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
      if (sub==subscription) {
        found = true;
      }
    }
  }
  while (lineIdx>-1);
  if (found==false) {
    mqtt_allSubscriptions= mqtt_allSubscriptions+subscription+"\n";
    client.subscribe(subscription.c_str());
    sendCommand("subscription added");
  }
  else {
    sendCommand("subscription exists");
  }
}

void unsubscribeCommand (String subscription) {
  if (!client.connected()) {
    sendCommand("mqtt not connected");
    return;
  }
  int start = 0;
  int lineIdx;
  String sub;
  String newAllSubscriptions = "";
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
      if (sub!=subscription) {
        newAllSubscriptions = newAllSubscriptions+sub+"\n";
      }
    }
  }
  while (lineIdx>-1);
  mqtt_allSubscriptions = newAllSubscriptions;
  client.unsubscribe(subscription.c_str());
  sendCommand("subscription removed");
}
