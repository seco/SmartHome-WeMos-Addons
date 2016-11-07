# SmartHome-WeMos-Addons
**Addons for SmartHome Controllers** based on WeMos D1 mini (or similar ESP8266 based development boards)

With this code and some hardware (cheap development boards and shields), you are able to create *connectedobjects* for [eq3 homematic CCU](http://www.eq-3.de/produkte/homematic.html) or any mqtt protocol based SmartHome controller. These __connectedobjects__ are small IoT bricks offering additional actuators or sensor funcionality to your SmartHome Controller. I prefer for mqtt [thethingbox] (http://www.thethingbox.com) as an universal mqtt server and nodered appliance.

----

First connectedobject is the __connectedobject HumiTemp__,
a temperature/humidity sensor in a HomeMatic and a mqtt version (keep an eye on the appendix)

Hardware:
* [WeMos d1 mini](https://www.wemos.cc/product/d1-mini.html) & [WeMos DHT11 shield](https://www.wemos.cc/product/dht-shield.html)


Software functions - Arduino Sketch:
* setup in Accesspoint Mode based on wifi manager library https://github.com/tzapu/WiFiManager
  (Wifi parameters, CCU IP Adress, sensors hostname, variables for humidity and temperature)
* the configuration is stored as json in fs
* http interface is announced via bonjour
* (HomeMatic) sensor data is transmitted to CCU and/or can be pulled via html requests
* (mqtt) sensor data is published to mqtt Server and/or can be pulled via html requests


Install:
* upload sketch to ESP8266
* ESP8266 starts in AP Mode, just connect and configure
* (HomeMatic) create two variables on CCU (same names as configured in the step before) for humidity and temperature

  hint: use name of room as prefix, in the case you plan to have more then one of these small guys
* (HomeMatic) use variables and values in own programs on CCU
* if your mqtt server needs username and password, please configure them in the initial step, if not, just leave the fields empty
* (mqtt) use configured topics in your programs
* http://ip-adress for mobile friendly website (see screenshot01.PNG)
* http://ip-adress/status for status (see screenshot02.PNG)
* http://ip-adress/help for help (see screenshot03.PNG)
* OTA Update http://ip-adress/firmware if you are using your sensor in a non save environment, comment out the line
```
  httpUpdater.setup(&server, update_path, update_username, update_password);
```
or your sensor will end as a zombie in a botnet. I prefer to have my sensors in a separate network with firewall and a jumphost to configure them. Please configure the update password during the initial setup when your sensor is in AP Mode. The username is admin.

Todos:
* thingspeak as an visualization alternative
* addon for presence recognition, to use give presence information to ccu - e.g. heating only if room is used.
* addon for [WeMos Battery shield](https://www.wemos.cc/product/battery-shield.html)
* 3d printable enclosure, with/without space for LiPo
* additional **connectedobjects**: wifi switch, OLED status display, motion detector, light barrier etc.

Usage:
* (HomeMatic) push: update variables on smarthome controller via html push (Sensor sends to Controller) and/or
* (mqtt) publish: publish topics to mqtt server and/or
* (HomeMatic) pull: update or metering parameters can be read (and set) via pull requests (Controller asks for updates) and/or
* webinterface for humans

Pictures:
mobile friendly website

![](https://github.com/holgerimbery/SmartHome-WeMos-Addons/blob/master/screens/screenshot01.PNG)

status screen

![](https://github.com/holgerimbery/SmartHome-WeMos-Addons/blob/master/screens/screenshot02.PNG)

help screen for html pull request from smarthome controller

![](https://github.com/holgerimbery/SmartHome-WeMos-Addons/blob/master/screens/screenshot03.PNG)

initial setup (Homematic Version)

![](https://github.com/holgerimbery/SmartHome-WeMos-Addons/blob/master/screens/initial_setup01.png)

![](https://github.com/holgerimbery/SmartHome-WeMos-Addons/blob/master/screens/initial_setup02.png)

This project is inspired by similar projects of the wemos and homematic communities
