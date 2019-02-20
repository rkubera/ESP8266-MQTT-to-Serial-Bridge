/*
 * ESP8266 MQTT Wifi Client to Serial Bridge with NTP
 * Author: rkubera https://github.com/rkubera/
 * License: MIT
 */
 
#include <String.h>

bool getSerialBuffer (char* buf, int &idx) {
  idx = 0;
  bool ret = false;
  while(true) {
    if(Serial.available()) {
      ret = true;
      char readed = Serial.read();
      buf[idx] = readed;
      buf[idx+1] = 0;
      idx++;
      if((idx+1)>=bufferSize) break;
    } else break;
  }
  return ret;
}

void parseLines(char* buf) {
  static String myBuff = "";
  myBuff = myBuff+ buf;
  int start = 0;
  String line;
  String command;
  String payload;
  int lineIdx;
  int tmpIdx;
  do {
    lineIdx = myBuff.indexOf('\n', start);
    if (lineIdx>-1) {
      line = myBuff.substring(start, lineIdx);
      start = lineIdx+1;
      line.trim();
    }
    else {
      line = "";
    }
    
    if (line.length()>0) {
      tmpIdx = line.indexOf(' ');
      if (tmpIdx>-1) {
        command = line.substring(0, tmpIdx);
        payload = line.substring(tmpIdx+1);
      }
      else {
        command =  line;
        payload = "";
      }

      if (command=="connect") {
        connectCommand(payload);
      }
      else if (command=="mqttserver") {
        mqttServerCommand(payload);
      }
      else if (command=="mqttuserpass") {
        mqttUserPassCommand(payload);
      }
      else if (command=="subscribe") {
        subscribeCommand(payload);
      }
      else if (command=="unsubscribe") {
        unsubscribeCommand(payload);
      }
      else if (command=="publish") {
        publishCommand(payload);
      }
      else if (command=="publishretained") {
        publishretainedCommand(payload);
      }
      else if (command=="get") {
        getCommand(payload);
      }
      else {
        sendCommand("error");
      }
    }
  }
  while (lineIdx>-1);
  if (start>0) {
    myBuff = myBuff.substring(start);
  }
}


