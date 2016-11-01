# SmartHome-WeMos-Addons
**Smarthome Addons for Homematic** based on WeMos D1 mini (ESP8266)

With this code and some hardware you can create *connectedobjects* for [eq3 homematic CUU](http://www.eq-3.de/produkte/homematic.html)

temperature/humidity sensor for ccu
* hardware: [WeMos d1 mini](https://www.wemos.cc/product/d1-mini.html)& [WeMos DHT11 shield](https://www.wemos.cc/product/dht-shield.html)

Arduino Sketch:
* autosetup with wifi manager 
  (Wifi parameters, CCU IP Adress, variables for humidity and temperature) 
* config is stored as json


Install:
* upload sketch to ESP8266
* ESP8266 starts in AP Mode, connect and configure
* create to variables on CCU (same names as configured in the step before)
* use variables in own programs on CCU


Todos:
* proper webinterface
* addon for [WeMos Battery shield](https://www.wemos.cc/product/battery-shield.html) LiPo 
* additional **connectedobjects**




  
 
