/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  
 *   http://www.apache.org/licenses/LICENSE-2.0
 *   
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * (c) Holger Imbery, contact@connectedobjects.cloud 
 */


#include <FS.h>                   
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include "DHT.h"
#include "config.h"
ESP8266WebServer server(80);


//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

unsigned long dReadSeconds = 60;
float cTemp = 0;
float cHumi = 0; 

unsigned long dMessageSeconds = 3600;
float dTemp = 1;
float dHumi = 0;


float humidity = 0, humidityCCU = 0, temp = 0, tempCCU = 0;
unsigned long nowMillis = 0;
unsigned long dReadMillis = dReadSeconds * 1000, lastReadMillis = 0;
unsigned long dMessageMillis = dMessageSeconds * 1000, lastMessageMillis = 0;
String answer = "", message = "",lastmessageCCUtime = "";


String timestamp() { 
  char stamp[10];
  int currenth = millis()/3600000;
  int currentm = millis()/60000-currenth*60;
  int currents = millis()/1000-currenth*3600-currentm*60;
  sprintf (stamp,"%03d:%02d:%02d", currenth, currentm, currents);
  return stamp;
}

void metering() {
  humidity = dht.readHumidity() + cHumi;
    temp = dht.readTemperature() + cTemp;
    if (isnan(humidity) || isnan(temp)) {
     Serial.println(timestamp() + "  error: no data from sensor");
     delay(100);
     return;
    }
  lastReadMillis = nowMillis;
  Serial.println(timestamp() + "  humidity: " + humidity + " %  temperature: " + temp + " * C");
}

void message_CCU() { 
  WiFiClient client; 
  if (!client.connect(ip_adress_ccu, 8181)) { 
    Serial.println(timestamp() + "  error: no connection to CCU");
     delay(600);
    return;
  }
  message = "GET /eriwan.exe?answer1=dom.GetObject('" + String(variable_humi) + "').State('" + humidity + "')";
  message = message + "&answer2=dom.GetObject('" + String(variable_temp) + "').State('" + temp + "')";

  client.println(message); 
  delay(100);
  int i = 0;
  String line = "";
  
  while(client.available()){ 
    i ++;
    line = client.readStringUntil('\n');
    if (i == 7) {
      lastmessageCCUtime = line;
      
    }
  }
  Serial.println(timestamp() + "  data writen to ccu " + lastmessageCCUtime);
  lastMessageMillis = nowMillis;
  humidityCCU = humidity;
  tempCCU = temp;
} 

void message_status() { 
  String uptime = timestamp();
    String cssClass = "mediumhot";
    String message = "<!DOCTYPE html><html><head><title>status</title><meta charset=\"utf-8\" /><meta name=\"viewport\" content=\"width=device-width\" /><link href='https://fonts.googleapis.com/css?family=Advent+Pro' rel=\"stylesheet\" type=\"text/css\"><style>\n";
    message += "html {height: 100%;}";
    message += "div {color: #fff;font-family: 'Advent Pro';font-weight: 400;left: 50%;position: absolute;text-align: center;top: 50%;transform: translateX(-50%) translateY(-50%);}";
    message += "h2 {font-size: 90px;font-weight: 400; margin: 0}";
    message += "body {height: 100%;}";
    message += ".cold {background: linear-gradient(to bottom, #7abcff, #0665e0 );}";
    message += ".mediumhot {background: linear-gradient(to bottom, #81ef85,#057003);}";
    message += ".hot {background: linear-gradient(to bottom, #fcdb88,#d32106);}";
    message += "</style></head><body class=\"" + cssClass + "\"><div><h1>Status</h1>";
    message += "<h4>uptime: " + uptime + " (hhh:mm:ss)<p>";
    message += "last metering<br>";
    message += "temperature: " + String(temp) + " * C<br>";
    message += "humidity: " + String(humidity) + " %\<p>";
    message += "metering correction values<br>";
    message += "temperatur: " + String(cTemp) + " * C<br>";
    message += "humidity: " + String(cHumi) + " %<p>";
    message += "smarthome conroller updates<br>";
    message += "last transmission " + lastmessageCCUtime + "<br>";
    message += "written temperature: " + String(tempCCU) + " * C<br>";
    message += "written humidity: " + String(humidityCCU) + " %<p>";
    message += "trigger for data transfers<br>";
    message += "timeinterval: " + String(dMessageSeconds) + " seconds<br>";
    message += "delta temperature: " + String(dTemp) + " * C<br>";
    message += "delta humidity: " + String(dHumi) + " %<p>";
    message += "</h4></div></body></html>";
    server.send(200, "text/html", message);
}
  
void message_humidity() {
  String delta = server.arg("delta");
  String corr = server.arg("corr");
  if (delta != "") {
    dHumi = delta.toFloat();
    server.send(200, "text/plain", "humidity trigger value " + String(dHumi) + " % updated");
    delay(100);
    Serial.println(timestamp() + "  humidity trigger value " + dHumi + " %");
  }
  else if (corr != "") {
    cHumi = corr.toFloat();
    server.send(200, "text/plain", "correction factor for humidity " + String(cHumi) + " % updated");
    delay(100);
    Serial.println(timestamp() + "  correction factor for humidity " + cHumi + " %");
  }
  else {
    server.send(200, "text/plain", String(humidity));
    delay(100);
    Serial.println(timestamp() + "  humidity presented for pull");
  } 
}

void message_temp() {
  String delta = server.arg("delta");
  String corr = server.arg("corr");
  if (delta != "") {
    dTemp = delta.toFloat();
    server.send(200, "text/plain", "temperature trigger value " + String(dTemp) + " * C updated");
    delay(100);
    Serial.println(timestamp() + "  temperature trigger value " + dTemp + " * C");
  }
  else if (corr != "") {
    cTemp = corr.toFloat();
    server.send(200, "text/plain", "correction factor for temperature " + String(cTemp) + " * C updated");
    delay(100);
    Serial.println(timestamp() + "  correction factor for humidity " + cTemp + " * C");
  }
  else {
    server.send(200, "text/plain", String(temp));
    delay(100);
    Serial.println(timestamp() + "  temperature presented for pull");
  }
}


void message_root(){
  float temperature = temp;
  //float humidity = humidity;
  char temperatureString[6];
  char humidityString[11];
  dtostrf(temperature, 2, 2, temperatureString);
  dtostrf(humidity, 2, 2, humidityString);
  
    String title = "Temperature";
    String cssClass = "mediumhot";
     String subtitle = "Humidity";
  
    if (temperature < 0)
      cssClass = "cold";
    else if (temperature > 20)
      cssClass = "hot";
  
    String message = "<!DOCTYPE html><html><head><title>" + title + "</title><meta charset=\"utf-8\" /><meta name=\"viewport\" content=\"width=device-width\" /><link href='https://fonts.googleapis.com/css?family=Advent+Pro' rel=\"stylesheet\" type=\"text/css\"><style>\n";
    message += "html {height: 100%;}";
    message += "div {color: #fff;font-family: 'Advent Pro';font-weight: 400;left: 50%;position: absolute;text-align: center;top: 50%;transform: translateX(-50%) translateY(-50%);}";
    message += "h2 {font-size: 90px;font-weight: 400; margin: 0}";
    message += "body {height: 100%;}";
    message += ".cold {background: linear-gradient(to bottom, #7abcff, #0665e0 );}";
    message += ".mediumhot {background: linear-gradient(to bottom, #81ef85,#057003);}";
    message += ".hot {background: linear-gradient(to bottom, #fcdb88,#d32106);}";
    message += "</style></head><body class=\"" + cssClass + "\"><div><h1>" + title +  "</h1><h2>" + temperatureString + "&nbsp;<small>&deg;C</small> <p> <h1>" + subtitle +  "</h1><h2>" + humidityString + "&nbsp;<small>&#37;</small></h2></div>";
    message += "</body></html>";
    
    server.send(200, "text/html", message);
}
void message_help() {

    String cssClass = "mediumhot";
String message = "<!DOCTYPE html><html><head><title> Help</title><meta charset=\"utf-8\" /><meta name=\"viewport\" content=\"width=device-width\" /><link href='https://fonts.googleapis.com/css?family=Advent+Pro' rel=\"stylesheet\" type=\"text/css\"><style>\n";
    message += "html {height: 100%;}";
    message += "div {color: #fff;font-family: 'Advent Pro';font-weight: 400;left: 50%;position: absolute;text-align: center;top: 50%;transform: translateX(-50%) translateY(-50%);}";
    message += "h2 {font-size: 25px;font-weight: 400; margin: 0}";
    message += "body {height: 100%;}";
    message += ".cold {background: linear-gradient(to bottom, #7abcff, #0665e0 );}";
    message += ".mediumhot {background: linear-gradient(to bottom, #81ef85,#057003);}";
    message += ".hot {background: linear-gradient(to bottom, #fcdb88,#d32106);}";
    message += "</style></head><body class=\"" + cssClass + "\"><div><h2>Help</h2>";
    message += "<h5>&lt;IP-Adresse&gt;&#47;temp&#09;&#09;returns temperature in (Celsius)<p>";
    message += "&lt;IP-Adresse&gt;&#47;humidity&#09;&#09;returns humidity in (percent)<p>";
    message += "&lt;IP-Adresse&gt;&#47;time&#09;&#09;returns time of last smarthome controller update<p>";
    message += "&lt;IP-Adresse&gt;&#47;temp?delta=&lt;value&gt;&#09;&#09;to set new value for temperature update trigger<p>";
    message += "&lt;IP-Adresse&gt;&#47;temp?corr=&lt;value&gt;&#09;&#09;to set a correction factor for temperature<p>";
    message += "&lt;IP-Adresse&gt;&#47;humidity?delta=&lt;value&gt;&#09;&#09;to set new value for humidity update trigger<p> ";
    message += "&lt;IP-Adresse&gt;&#47;humidity?corr=&lt;value&gt;&#09;&#09;to set a correction factor for humidity<p>";
    message += "&lt;IP-Adresse&gt;&#47;time?delta=&lt;value&gt;&#09;&#09;to set interval for smarthome controller update (s)<p>";
    message += "&lt;IP-Adresse&gt;&#47;time?meter=&lt;value&gt;&#09;&#09;to set interval for metering (s)</h5></div>";
    message += "</body></html>";
    
    server.send(200, "text/html", message);
}
void message_time() {
  String delta = server.arg("delta");
  String meter = server.arg("meter");
  if (delta != "") {
    dMessageSeconds = delta.toInt();
    dMessageMillis = dMessageSeconds * 1000;
    server.send(200, "text/plain", "updated interval for ccu update to " + delta + " seconds");
    delay(100);
    Serial.println(timestamp() + "  updated interval for ccu update to " + delta + " seconds");
  }
  else if (meter != "") {
    dReadMillis = meter.toInt() * 1000;
    server.send(200, "text/plain", "update metering interval to " + meter + " seconds");
    delay(100);
    Serial.println(timestamp() + "  update metering interval to " + meter + " seconds");
  }
  else {
    server.send(200, "text/plain", "last ccu update\n" + lastmessageCCUtime);
    delay(100);
    Serial.println(timestamp() + "  last ccu update ");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println(timestamp() + "  mounting FS...");

  if (SPIFFS.begin()) {
  Serial.println(timestamp() + "  mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println(timestamp() + "  reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println(timestamp() + "  opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
            Serial.println(timestamp() + "  parsed json");

          strcpy(ip_adress_ccu, json["ip_adress_ccu"]);
          strcpy(variable_temp, json["variable_temp"]);
          strcpy(variable_humi, json["variable_humi"]);
        } else {
            Serial.println(timestamp() + "  failed to load json config");
        }
      }
    }
  } else {
      Serial.println(timestamp() + "  failed to mount FS");
  }
  //end read



  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_ip_adress_ccu("ccu", "Static IP CCU", ip_adress_ccu, 40);
  WiFiManagerParameter custom_variable_temp("temp", "temperature variable", variable_temp, 40);
  WiFiManagerParameter custom_variable_humi("humi", "humidity variable", variable_humi, 40);
  //WiFiManager
 
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  IPAddress _ip,_gw,_sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);
  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  
  //add all your parameters here
  wifiManager.addParameter(&custom_ip_adress_ccu);
  wifiManager.addParameter(&custom_variable_temp);
  wifiManager.addParameter(&custom_variable_humi);
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimum quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point
  if (!wifiManager.autoConnect("ConnectedObjects_HumiTemp")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  
  Serial.println(timestamp() + "  connected...");

  
  strcpy(ip_adress_ccu, custom_ip_adress_ccu.getValue());
  Serial.println(timestamp() + "  CCU IP Adress: " + String(ip_adress_ccu));
  strcpy(variable_temp, custom_variable_temp.getValue());
  strcpy(variable_humi, custom_variable_humi.getValue());


  if (shouldSaveConfig) {
    Serial.println(timestamp() + "  saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["ip_adress_ccu"] = ip_adress_ccu;
    json["variable_temp"] = variable_temp;
    json["variable_humi"] = variable_humi;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println(timestamp() + "  failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();

  }

  server.on("/", message_root);
  server.on("/temp", message_temp);
  server.on("/humidity", message_humidity);
  server.on("/time", message_time);
  server.on("/status", message_status);
  server.on("/help", message_help);

  server.begin();

  lastReadMillis = millis();
  lastMessageMillis = millis();

}

// main program
void loop() {
  server.handleClient();
  nowMillis = millis();

  if(nowMillis - lastReadMillis > dReadMillis) {
    Serial.println(timestamp() + "  time to get new values from sensor");
    metering();
  }

  if(!dMessageMillis == 0 && nowMillis - lastMessageMillis > dMessageMillis) {
    message_CCU();
  }
   
   if(!dHumi == 0 && abs(humidity - humidityCCU) >= dHumi) {
    Serial.println(timestamp() + "  send humidity value to ccu");    
    message_CCU();
  }

  if(!dTemp == 0 && abs(temp - tempCCU) >= dTemp) { 
    Serial.println(timestamp() + "  send temperature value to ccu"); 
    message_CCU();
  }
}
