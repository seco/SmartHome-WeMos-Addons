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


#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include "DHT.h"
#include "config.h"
ESP8266WebServer server(80);
DHT dht(D4, DHT11);

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
void root_action() { 
  String uptime = timestamp();
  
  
  
  answer ="connectedobjects temperature-/humidity sensor\n";
  answer += "\tuptime: " + uptime + " (hhh:mm:ss)\n\n";
  answer += "last data\n\ttemperature: " + String(temp) + " * C\n";
  answer += "\thumidity: " + String(humidity) + " %\n\n";
  answer += "correction \n\ttemperatur: " + String(cTemp) + " * C\n";
  answer += "\thumidity: " + String(cHumi) + " %\n\n";
  answer += "last transmission to ccu\n\t" + lastmessageCCUtime;
  answer += "\twritten temperature: " + String(tempCCU) + " * C\n";
  answer += "\twritten humidity: " + String(humidityCCU) + " %\n\n";
  answer += "trigger for data transfer to CCU\n";
  answer += "\ttimeinterval: " + String(dMessageSeconds) + " seconds\n";
  answer += "\tdelta temperature: " + String(dTemp) + " * C\n";
  answer += "\tdelta humidity: " + String(dHumi) + " %\n\n\n";
  answer += "Menu:\n";
  answer += "\"<IP-Adresse>/temp\"\n\ttemperature in (Celsius)\n";
  answer += "\"<IP-Adresse>/humidity\"\n\thumidity in (percent)\n";
  answer += "\"<IP-Adresse>/time\"\n\ttimestamp last ccu update\n";
  answer += "\"<IP-Adresse>/temp?delta=<value>\"\n\tadjust temperature trigger value\n";
  answer += "\"<IP-Adresse>/temp?corr=<value>\"\n\tadjust correction factor for temperature\n";
  answer += "\"<IP-Adresse>/humidity?delta=<value>\"\n\tadjust humidity trigger value\n";
  answer += "\"<IP-Adresse>/humidity?corr=<value>\"\n\tadjust correction factor for humidity\n";
  answer += "\"<IP-Adresse>/time?delta=<value> \"\n\tadjust interval for ccu update (s)\n";
  answer += "\"<IP-Adresse>/time?meter=<value>\"\n\tadjust metering interval (s)\n";
  server.send(300, "text/raw", answer);
  delay(150);
  Serial.println(timestamp() + "  non specific call");
}

void message_humidity() {
  String delta = server.arg("delta");
  String corr = server.arg("corr");
  if (delta != "") {
    dHumi = delta.toFloat();
    server.send(200, "text/plain", "humidity trigger value " + delta + " % updated");
    delay(100);
    Serial.println(timestamp() + "  humidity trigger value " + delta + " %");
  }
  else if (corr != "") {
    cHumi = corr.toFloat() * 1000;
    server.send(200, "text/plain", "correction factor for humidity " + corr + " % updated");
    delay(100);
    Serial.println(timestamp() + "  correction factor for humidity " + corr + " %");
  }
  else {
    server.send(200, "text/plain", String(humidity));
    delay(100);
    Serial.println(timestamp() + "  humidity updated on ccu");
  } 
}

void message_temp() {
  String delta = server.arg("delta");
  String corr = server.arg("corr");
  if (delta != "") {
    dTemp = delta.toFloat();
    server.send(200, "text/plain", "temperature trigger value " + delta + " * C updated");
    delay(100);
    Serial.println(timestamp() + "  temperature trigger value " + delta + " * C");
  }
  else if (corr != "") {
    cTemp = corr.toFloat() * 1000;
    server.send(200, "text/plain", "correction factor for temperature " + corr + " * C updated");
    delay(100);
    Serial.println(timestamp() + "  correction factor for humidity " + corr + " * C");
  }
  else {
    server.send(200, "text/plain", String(temp));
    delay(100);
    Serial.println(timestamp() + "  temperature updated on ccu");
  }
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
    Serial.println(timestamp() + "  latest ccu update ");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(ip_adress_ccu, json["ip_adress_ccu"]);
          strcpy(variable_temp, json["variable_temp"]);
          strcpy(variable_humi, json["variable_humi"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
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

  //if you get here you have connected to the WiFi
  Serial.println(timestamp() + "  connected...");

  //read updated parameters
  strcpy(ip_adress_ccu, custom_ip_adress_ccu.getValue());
  Serial.println(timestamp() + "  CCU IP Adress: " + String(ip_adress_ccu));
  strcpy(variable_temp, custom_variable_temp.getValue());
  strcpy(variable_humi, custom_variable_humi.getValue());


  //save the custom parameters to FS
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
    //end save
  }

  server.on("/", root_action);
  server.on("/temp", message_temp);
  server.on("/humidity", message_humidity);
  server.on("/time", message_time);
  

  server.begin();

  lastReadMillis = millis();
  lastMessageMillis = millis();

}

// main program
void loop() {
  // auf HTTP-Anfragen warten
  server.handleClient();
  nowMillis = millis();

  if(nowMillis - lastReadMillis > dReadMillis) {
    metering();
  }

  if(!dMessageMillis == 0 && nowMillis - lastMessageMillis > dMessageMillis) {
    message_CCU();
  }
   
   if(!dHumi == 0 && abs(humidity - humidityCCU) >= dHumi) {
    message_CCU();
  }

  if(!dTemp == 0 && abs(temp - tempCCU) >= dTemp) { 
    message_CCU();
  }
}
