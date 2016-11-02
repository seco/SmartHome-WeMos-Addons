# SmartHome-WeMos-Addons
**Addons for Homematic SmartHome Controller** based on WeMos D1 mini (or similar ESP8266 based development boards)

With this code and some hardware, you can create *connectedobjects* for [eq3 homematic CCU](http://www.eq-3.de/produkte/homematic.html)

__connectedobject HumiTemp__
temperature/humidity sensor for ccu

* hardware: [WeMos d1 mini](https://www.wemos.cc/product/d1-mini.html) & [WeMos DHT11 shield](https://www.wemos.cc/product/dht-shield.html)


Arduino Sketch:
* autosetup with wifi manager 
  (Wifi parameters, CCU IP Adress, variables for humidity and temperature) 
* config is stored as json
* small managment interface via web-browser


Install:
* upload sketch to ESP8266
* ESP8266 starts in AP Mode, just connect and configure
* create two variables on CCU (same names as configured in the step before) for humidity and temperature

   hint: use name of room as prefix, in the case you plan to have more then one of these small guys
* use variables and values in own programs on CCU
* http://ip-adress for mobile friendly website (see screenshot01.PNG)
* http://ip-adress/status for status and additional commands

Todos:
* proper webinterface for status screen
* 3d printable enclosure
* addon for [WeMos Battery shield](https://www.wemos.cc/product/battery-shield.html)
* additional **connectedobjects**: wifi switch, OLED display, motion detector, etc.

Pictures:
mobile friendly website

![](https://github.com/holgerimbery/SmartHome-WeMos-Addons/blob/master/screens/screenshot01.PNG)

status screen

![](https://github.com/holgerimbery/SmartHome-WeMos-Addons/blob/master/screens/screenshot02.PNG)
