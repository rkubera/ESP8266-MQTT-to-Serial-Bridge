/*
 * ESP8266 MQTT Wifi Client to Serial Bridge with NTP
 * Author: rkubera https://github.com/rkubera/
 * License: MIT
 */
 
#include <String.h>

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

void commandLoop() {
  if (getSerialBuffer((char*)myBuffer, bufIdx)) {
    parseLines((char*)myBuffer);
  }
  parseBuffer();
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
    if (wificonnected==true) {
      sendCommand("wifi connected");
    }
    else {
      sendCommand("wifi not connected");
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
    sendCommand(ssid);
  } 
  else if (payload=="mqttserver") {
    sendCommand(mqtt_server);
  }
  else if (payload=="mqttport") {
    sendCommand((String)mqtt_port);
  }
  else if (payload=="mqttuser") {
    sendCommand(mqtt_user);
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
void connectCommand(String payload) {
  int tmpIdx;
  tmpIdx = payload.indexOf(':');
  if (tmpIdx>-1) {
    //AP ssid and password
    ssid = payload.substring(0, tmpIdx);
    password = payload.substring(tmpIdx+1);
  }
  else {
    //open AP, only ssid
    ssid = payload;
  }
  client.disconnect();
  WiFi.disconnect();
  wificonnected= false;
  sendCommand("connecting to wifi");
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
void mqttUserPassCommand(String payload) {
  int tmpIdx;
  tmpIdx = payload.indexOf(':');
  if (tmpIdx>-1) {
    //mqtt server and port
    mqtt_user = payload.substring(0, tmpIdx);
    mqtt_pass = payload.substring(tmpIdx+1);
  }
  else {
    //mqtt only
    mqtt_user = payload;
  }
  client.disconnect();
  sendCommand("mqtt user and pass set");
}

void mqttServerCommand(String payload) {
  int tmpIdx;
  tmpIdx = payload.indexOf(':');
  if (tmpIdx>-1) {
    //mqtt server and port
    mqtt_server = payload.substring(0, tmpIdx);
    mqtt_port = payload.substring(tmpIdx+1).toInt();
  }
  else {
    //mqtt only
    mqtt_server = payload;
  }
  client.disconnect();
  client.setServer(mqtt_server.c_str(), mqtt_port);
  sendCommand("connecting to mqtt server");
}

void subscribeCommand (String subscription) {
  if (!client.connected()) {
    sendCommand("mqtt not connected");
    return;
  }
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
