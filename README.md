# SmartHome-WeMos-Addons
**Addons for Homematic SmartHome Controller** based on WeMos D1 mini (or similar ESP8266 based development boards)

With this code and some hardware (cheap development boards and shields), you are able to create *connectedobjects* for [eq3 homematic CCU](http://www.eq-3.de/produkte/homematic.html). These __connectedobjects__ are small IoT bricks offering additional actuators or sensor funcionality to your SmartHome Controller.  

First connectedobject is the
__connectedobject HumiTemp__ 
a temperature/humidity sensor for the ccu

Hardware:
* [WeMos d1 mini](https://www.wemos.cc/product/d1-mini.html) & [WeMos DHT11 shield](https://www.wemos.cc/product/dht-shield.html)


Software - Arduino Sketch:
* Setup in Accesspoint Mode based on wifi manager library https://github.com/tzapu/WiFiManager
  (Wifi parameters, CCU IP Adress, variables for humidity and temperature) 
* the configuration is stored as json in fs
* sensor data is transmitted to CCU and/or can be pulled via html requests


Install:
* upload sketch to ESP8266
* ESP8266 starts in AP Mode, just connect and configure
* create two variables on CCU (same names as configured in the step before) for humidity and temperature

   hint: use name of room as prefix, in the case you plan to have more then one of these small guys
* use variables and values in own programs on CCU
* http://ip-adress for mobile friendly website (see screenshot01.PNG)
* http://ip-adress/status for status (see screenshot02.PNG)
* http://ip-adress/help for help (see screenshot03.PNG)

Todos:
* addon for presence recognition, to use give presence information to ccu - e.g. heating only if room is used.
* addon for [WeMos Battery shield](https://www.wemos.cc/product/battery-shield.html)
* 3d printable enclosure, with/without space for LiPo
* additional **connectedobjects**: wifi switch, OLED statusdisplay, motion detector, light barrier etc.

Usage:
* push: updates variables on smarthome controller via html push (Sensor sends to Controller) and/or
* pull: updates or metering parameters can be read (and set) via pull requests (Controller asks for updates) and/or
* webinterface for humans

Pictures:
mobile friendly website

![](https://github.com/holgerimbery/SmartHome-WeMos-Addons/blob/master/screens/screenshot01.PNG)

status screen

![](https://github.com/holgerimbery/SmartHome-WeMos-Addons/blob/master/screens/screenshot02.PNG)

help screen for html pull request from smarthome controller

![](https://github.com/holgerimbery/SmartHome-WeMos-Addons/blob/master/screens/screenshot03.PNG)

initial setup

![](https://github.com/holgerimbery/SmartHome-WeMos-Addons/blob/master/screens/initial_setup01.png)

![](https://github.com/holgerimbery/SmartHome-WeMos-Addons/blob/master/screens/initialsetup02.png)

This project is inspired by similar projects of the wemos and homematic communities
