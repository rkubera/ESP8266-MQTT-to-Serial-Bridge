#ESP8266 MQTT Wifi Client to Serial Bridge with NTP

##License: MIT

##Features
- Connect to MQTT Server via wifi
- Automaticaly get NTP time
- Automaticaly reconnect to Wifi if signal lost
- Automaticaly reconnect and resubscribe to MQTT server if disconnected
- Easy API

##Connection
Connect do ESP8266 via UART port (57600 baudrate)

##API
Commands (must be finished with \n EOL):
1) connect
1a) Connect to open accespoint myAccesspoint
- command: connect myAccespoint
- response1 (on start): [conecting to wifi]
- response2 (on success): [wifi connected]
1b) Connect to accespoint myAccesspoint with myPassword
- command: connect myAccespoint:myPassword
- response1 (on start): [conecting to wifi]
- response2 (on success): [wifi connected]

2) mqttuserpass
2a) Set Mqtt Server user mymqttuser with empty pass
- command: mqttuserpass mymqttuser
- response: [mqtt user and pass set]
2b) Set Mqtt Server user mymqttuser with password mymqttpass
- command: mqttuserpass mymqttuser:mymqttpass
- response: [mqtt user and pass set]

3) mqttserver
3a) Set Mqtt Server mqttserver mymqttserver only (port will be set as Default:1883)
- command: mqttserver mymqttserver
- response1 (on start): [connecting to mqtt server]
- response2 (on success): [mqtt server connected]
3b) Set Mqtt Server mqttserver mymqttuser and mqttport 1885
- command: mqttserver mymqttserver:1885
- response1 (on start): [connecting to mqtt server]
- response2 (on success): [mqtt server connected]

4) publish and publishretained
4a) Publish topic mytopic with payload mypayload:
- command: publish mytopic mypayload
- response: [published] or [wrong publish command] or [mqtt server not connected]
4b) Publish retained topic mymtopic with payload mypayload:
- command: publishretained mytopic mypayload
- response: [published] or [wrong publish command] or [mqtt server not connected]

5) subscribe and unsubscribe
5a) Subsctibe topic mytopic
- command: subscribe mytopic
- response: [subscription added] or [subscription already exists] or [mqtt server not connected]
5b) Unsubscribe topic mytopic
- command: unsubscribe mytopic
- response: [subscription removed] or [mqtt server not connected]

6) get
6a) Get timestamp from NTP (UTC+0)
- command: get timestamp
- response: [timestamp 1131334556] or [timestamp 0] if not connected (yet) to NTP server
6b) Get echo (check if ESP8266 is live)
- command: get echo
- response [echo]
6c) Get ip address
- command: get ip
- response: [123.123.123.123]
6d) Get WiFi status
- command: get wifistatus
- response: [connected] or [disconnected]
6e) Get ssid
- command: get ssid
- resonse [myssid]
6f) Get mqtt Server
- comand: get mqttserver
- response: [mymqttserver]
6g) Get mqtt user
- command: get mqttuser
- response: [mymqttuser]

7) errors
if ESP8266 not regognised command will return [error] message.

8) Quck example:
Send: connect mywifi:mypass
Recv: [connecting to wifi]
Recv: [wifi connected]
Send: mqttuserpass mymqttuser:mymqttpass
Recv: [mqtt user and pass set]
Send: mqttserver mymqtt
Recv: [connecting to mqtt server]
Recv: [mqttserver connected]
Send: subscribe espMessages/#
Recv: [subscription added]
Send: publish espMessages/test Hello World!
Recv: espMessages/test Hello World!
Send: get timestamp
Rrcv: [timestamp 1562556785]