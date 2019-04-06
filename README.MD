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
- CRC32 control

## Connection
Connect do ESP8266 via UART port (9600 baudrate)

## API
All responses starts with crc32 checksum, for example:
- 1426454121 [connecting to wifi]

In commands description crc32 checksum will be represented as CRC32 for eample:
- CRC32 [connecting to wifi]

Commands (must be finished with \n EOL):
1) <b>connect</b>
- Connect to open accespoint myAccesspoint
  - command: connect myAccespoint
  - response1 (on start): CRC32 [conecting to wifi]
  - response2 (on success): CRC32 [wifi connected]
- Connect to accespoint myAccesspoint with myPassword
  - command: connect myAccespoint:myPassword
  - response1 (on start): CRC32 [conecting to wifi]
  - response2 (on success): CRC32 [wifi connected]

2) <b>mqttuserpass</b>
- Set Mqtt Server user mymqttuser with empty pass
  - command: mqttuserpass mymqttuser
  - response: CRC32 [mqtt user and pass set]
- Set Mqtt Server user mymqttuser with password mymqttpass
  - command: mqttuserpass mymqttuser:mymqttpass
  - response: CRC32 [mqtt user and pass set]

3) <b>mqttserver</b>
- Set Mqtt Server mqttserver mymqttserver only (port will be set as Default:1883)
  - command: mqttserver mymqttserver
  - response1 (on start): CRC32 [connecting to mqtt server]
  - response2 (on success): CRC32 [mqtt connected]
- Set Mqtt Server mqttserver mymqttuser and mqttport 1885
  - command: mqttserver mymqttserver:1885
  - response1 (on start): CRC32 [connecting to mqtt server]
 - response2 (on success): CRC32 [mqtt connected]

4) <b>publish</b> and <b>publishretained</b>
- Publish topic mytopic with payload mypayload:
  - command: publish mytopic mypayload
  - response: CRC32 [published] or CRC32 [wrong publish command] or CRC32 [mqtt not connected]
- Publish retained topic mymtopic with payload mypayload:
  - command: publishretained mytopic mypayload
  - response: CRC32 [published] or CRC32 [wrong publish command] or CRC32 [mqtt not connected]

5) <b>subscribe</b> and <b>unsubscribe</b>
- Subsctibe topic mytopic
  - command: subscribe mytopic
  - response: CRC32 [subscription added] or CRC32 [mqtt not connected]
- Unsubscribe topic mytopic
  - command: unsubscribe mytopic
  - response: CRC32 [subscription removed] or CRC32 [mqtt not connected]

6) <b>get</b>
- Get timestamp from NTP (UTC+0)
  - command: get timestamp
  - response: CRC32 [timestamp 1131334556] or CRC32 [timestamp 0] if not connected to NTP server (yet)
- Get echo (check if ESP8266 is live)
  - command: get echo
  - response: CRC32 [echo]
- Get ip address
  - command: get ip
  - response: CRC32 [123.123.123.123]
- Get WiFi status
  - command: get wifistatus
  - response: CRC32 [wifi connected] or [wifi not connected]
- Get ssid
  - command: get ssid
  - resonse: CRC32 [myssid]
- Get MQTT Server status
  - command: get mqttstatus
  - response: CRC32 [mqtt connected] or CRC32 [mqtt not connected]
- Get mqtt Server
  - comand: get mqttserver
  - response: CRC32 [mymqttserver]
- Get mqtt user
  - command: get mqttuser
  - response: CRC32 [mymqttuser]

7) errors
if ESP8266 not recognised command will return [error] message.

# Quck example:
- Send: connect mywifi:mypass
- Recv: 1426454121 [connecting to wifi]
- Recv: 3286076215 [wifi connected]
- Send: mqttuserpass mymqttuser:mymqttpass
- Recv: 504100829 [mqtt user and pass set]
- Send: mqttserver mymqtt
- Recv: 3239127883 [connecting to mqtt server]
- Recv: 4088902839 [mqtt connected]
- Send: subscribe espMessages/#
- Recv: 294403144 [subscription added]
- Send: publish espMessages/test Hello World!
- Recv: 2549217433 [published]
- Recv: 1413984600 espMessages/test Hello World!
- Send: get timestamp
- Recv: 3638178531 [timestamp 1554581454]
