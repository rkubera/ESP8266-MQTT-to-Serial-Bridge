# ESP8266 MQTT Wifi Client to Serial Bridge with NTP
by rkubera (Radek Kubera) https://github.com/rkubera
- License: MIT

## Features
- Connect to MQTT Server via WIFI
- Publish Topics to MQTT Server via Serial
- Subscribe/Unsubscribe Topics from MQTT Server via Serial
- Automaticaly reconnect to Wifi if signal lost
- Automaticaly reconnect to MQTT Server and resubscribe Topics if disconnected
- Automaticaly get NTP time
- Easy API

## Connection
Connect do ESP8266 via UART port (57600 baudrate)

## API
Commands (must be finished with \n EOL):
1) <b>connect</b>
- Connect to open accespoint myAccesspoint
  - command: connect myAccespoint
  - response1 (on start): [conecting to wifi]
  - response2 (on success): [wifi connected]
- Connect to accespoint myAccesspoint with myPassword
  - command: connect myAccespoint:myPassword
  - response1 (on start): [conecting to wifi]
  - response2 (on success): [wifi connected]

2) <b>mqttuserpass</b>
- Set Mqtt Server user mymqttuser with empty pass
  - command: mqttuserpass mymqttuser
  - response: [mqtt user and pass set]
- Set Mqtt Server user mymqttuser with password mymqttpass
  - command: mqttuserpass mymqttuser:mymqttpass
  - response: [mqtt user and pass set]

3) <b>mqttserver</b>
- Set Mqtt Server mqttserver mymqttserver only (port will be set as Default:1883)
  - command: mqttserver mymqttserver
  - response1 (on start): [connecting to mqtt server]
  - response2 (on success): [mqtt server connected]
- Set Mqtt Server mqttserver mymqttuser and mqttport 1885
  - command: mqttserver mymqttserver:1885
  - response1 (on start): [connecting to mqtt server]
 - response2 (on success): [mqtt server connected]

4) <b>publish</b> and <b>publishretained</b>
- Publish topic mytopic with payload mypayload:
  - command: publish mytopic mypayload
  - response: [published] or [wrong publish command] or [mqtt server not connected]
- Publish retained topic mymtopic with payload mypayload:
  - command: publishretained mytopic mypayload
  - response: [published] or [wrong publish command] or [mqtt server not connected]

5) <b>subscribe</b> and <b>unsubscribe</b>
- Subsctibe topic mytopic
  - command: subscribe mytopic
  - response: [subscription added] or [subscription already exists] or [mqtt server not connected]
- Unsubscribe topic mytopic
  - command: unsubscribe mytopic
  - response: [subscription removed] or [mqtt server not connected]

6) <b>get</b>
- Get timestamp from NTP (UTC+0)
  - command: get timestamp
  - response: [timestamp 1131334556] or [timestamp 0] if not connected (yet) to NTP server
- Get echo (check if ESP8266 is live)
  - command: get echo
  - response [echo]
- Get ip address
  - command: get ip
  - response: [123.123.123.123]
- Get WiFi status
  - command: get wifistatus
  - response: [connected] or [disconnected]
- Get ssid
  - command: get ssid
  - resonse [myssid]
- Get mqtt Server
  - comand: get mqttserver
  - response: [mymqttserver]
- Get mqtt user
  - command: get mqttuser
  - response: [mymqttuser]

7) errors
if ESP8266 not regognised command will return [error] message.

# Quck example:
- Send: connect mywifi:mypass
- Recv: [connecting to wifi]
- Recv: [wifi connected]
- Send: mqttuserpass mymqttuser:mymqttpass
- Recv: [mqtt user and pass set]
- Send: mqttserver mymqtt
- Recv: [connecting to mqtt server]
- Recv: [mqttserver connected]
- Send: subscribe espMessages/#
- Recv: [subscription added]
- Send: publish espMessages/test Hello World!
- Recv: espMessages/test Hello World!
- Send: get timestamp
- Rrcv: [timestamp 1562556785]
