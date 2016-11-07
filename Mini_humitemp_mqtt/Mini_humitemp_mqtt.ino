/*

  The MIT License (MIT)

  Copyright (c) 2016 HOLGER IMBERY, CONTACT@CONNECTEDOBJECTS.CLOUD

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/


#include <FS.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include "DHT.h"
#include "config.h"
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
String ID = "ConnectedObjects-" + String(ESP.getChipId(),HEX);
const char * mdnsID = ID.c_str();


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


float humidity = 0, humiditymqtt = 0, temp = 0, tempmqtt = 0;
unsigned long nowMillis = 0;
unsigned long dReadMillis = dReadSeconds * 1000, lastReadMillis = 0;
unsigned long dMessageMillis = dMessageSeconds * 1000, lastMessageMillis = 0;
String answer = "", message = "";


String timestamp() {
  char stamp[10];
  int currenth = millis() / 3600000;
  int currentm = millis() / 60000 - currenth * 60;
  int currents = millis() / 1000 - currenth * 3600 - currentm * 60;
  sprintf (stamp, "%03d:%02d:%02d", currenth, currentm, currents);
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

void message_mqtt() {
  WiFiClient client;
  PubSubClient mqttclient(client);
  mqttclient.setServer(ip_adress_mqtt, 1883);


  if (!mqtt_user == "") {
    while (!mqttclient.connected()) {
      Serial.print(Serial.println(timestamp() + "  Attempting MQTT connection..."));
      // Attempt to connect
      // If you do not want to use a username and password, change next line to
      if (mqttclient.connect(hostname_sensor)) {
        // if (mqttclient.connect("ESP8266Client", mqtt_user, mqtt_password)) {
        Serial.print(Serial.println(timestamp() + "  connected"));
      } else {
        Serial.print(Serial.println(timestamp() + "  failed, rc="));
        Serial.print(Serial.println(timestamp() + mqttclient.state()));
        Serial.print(Serial.println(timestamp() + "  try again in 5 seconds"));
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  }
  else {
    while (!mqttclient.connected()) {
      Serial.print(Serial.println(timestamp() + "  Attempting MQTT connection..."));
      // Attempt to connect
      // If you do not want to use a username and password, change next line to
      //if (mqttclient.connect(hostname_sensor)) {
      if (mqttclient.connect("ESP8266Client", mqtt_user, mqtt_password)) {
        Serial.print(Serial.println(timestamp() + "  connected"));
      } else {
        Serial.print(Serial.println(timestamp() + "  failed, rc="));
        Serial.print(Serial.println(timestamp() + mqttclient.state()));
        Serial.print(Serial.println(timestamp() + "  try again in 5 seconds"));
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }

  }


  mqttclient.publish(temperature_topic, String(temp).c_str(), true);
  mqttclient.publish(humidity_topic, String(humidity).c_str(), true);
  Serial.print(Serial.println(timestamp() + " new humidity: " + String(humidity)));
  Serial.print(Serial.println(timestamp() + " new temperature: " + String(temp)));
  Serial.println(timestamp() + "  data writen to mqtt ");
  lastMessageMillis = nowMillis;
  humiditymqtt = humidity;
  tempmqtt = temp;
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
  message += "smarthome controller updates<br>";
  message += "written temperature: " + String(tempmqtt) + " * C<br>";
  message += "written humidity: " + String(humiditymqtt) + " %<p>";
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


void message_root() {
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
  message += "</style></head><body class=\"" + cssClass + "\"><div><h1>" + title +  "</h1><h2>" + temperatureString + "&nbsp;<small>&deg;C</small> <p> <h1>" + subtitle +  "</h1><h2>" + humidityString + "&nbsp;<small>&#37;</small></h2><p><p>";
  //message += "<FORM style=\"color:blue;font-size:10px;\" METHOD=\"LINK\" ACTION=\"/status\"/><INPUT TYPE=\"submit\" VALUE=\"system status\"/></FORM>";
  message += "</div></body></html>";

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
  message += "&lt;IP-Adresse&gt;&#47;temp?delta=&lt;value&gt;&#09;&#09;to set new value for temperature update trigger<p>";
  message += "&lt;IP-Adresse&gt;&#47;temp?corr=&lt;value&gt;&#09;&#09;to set a correction factor for temperature<p>";
  message += "&lt;IP-Adresse&gt;&#47;humidity?delta=&lt;value&gt;&#09;&#09;to set new value for humidity update trigger<p> ";
  message += "&lt;IP-Adresse&gt;&#47;humidity?corr=&lt;value&gt;&#09;&#09;to set a correction factor for humidity<p>";
  message += "&lt;IP-Adresse&gt;&#47;time?delta=&lt;value&gt;&#09;&#09;to set interval for mqtt server update (s)<p>";
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
    server.send(200, "text/plain", "updated interval for server update to " + delta + " seconds");
    delay(100);
    Serial.println(timestamp() + "  updated interval for server update to " + delta + " seconds");
  }
  else if (meter != "") {
    dReadMillis = meter.toInt() * 1000;
    server.send(200, "text/plain", "update metering interval to " + meter + " seconds");
    delay(100);
    Serial.println(timestamp() + "  update metering interval to " + meter + " seconds");
  }
  else {
    //server.send(200, "text/plain", "last ccu update\n" + lastmessageCCUtime);
    //delay(100);
    //Serial.println(timestamp() + "  last ccu update ");
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
          strcpy(hostname_sensor, json["hostname_sensor"]);
          strcpy(ip_adress_mqtt, json["ip_adress_mqtt"]);
          strcpy(temperature_topic, json["temperature_topic"]);
          strcpy(humidity_topic, json["humidity_topic"]);
          strcpy(humidity_topic, json["mqtt_user"]);
          strcpy(humidity_topic, json["mqtt_password"]);
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
  WiFiManagerParameter custom_hostname_sensor("hostnames", "Hostname Sensor", hostname_sensor, 40);
  WiFiManagerParameter custom_ip_adress_mqtt("mqtt", "Static IP mqtt server", ip_adress_mqtt, 40);
  WiFiManagerParameter custom_temperature_topic("temp", "temperature topic", temperature_topic, 40);
  WiFiManagerParameter custom_humidity_topic("humi", "humidity topic", humidity_topic, 40);
  WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 40);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 40);
  //WiFiManager

  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  IPAddress _ip, _gw, _sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);
  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);

  //add all your parameters here
  wifiManager.addParameter(&custom_hostname_sensor);
  wifiManager.addParameter(&custom_ip_adress_mqtt);
  wifiManager.addParameter(&custom_temperature_topic);
  wifiManager.addParameter(&custom_humidity_topic);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_password);
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

  strcpy(hostname_sensor, custom_hostname_sensor.getValue());
  strcpy(ip_adress_mqtt, custom_ip_adress_mqtt.getValue());
  Serial.println(timestamp() + "  mqtt IP Adress: " + String(ip_adress_mqtt));
  strcpy(temperature_topic, custom_temperature_topic.getValue());
  strcpy(humidity_topic, custom_humidity_topic.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_password, custom_mqtt_password.getValue());

  if (shouldSaveConfig) {
    Serial.println(timestamp() + "  saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["hostname_sensor"] = hostname_sensor;
    json["ip_adress_mqtt"] = ip_adress_mqtt;
    json["temperature_topic"] = temperature_topic;
    json["humidity_topic"] = humidity_topic;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_password"] = mqtt_password;
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
  server.on("/time", message_time);
  server.on("/humidity", message_humidity);
  server.on("/status", message_status);
  server.on("/help", message_help);

  server.begin();
  httpUpdater.setup(&server);

  if (!MDNS.begin(mdnsID)) {
    Serial.println(timestamp() + "  Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println(timestamp() + "  Host: " + mdnsID + "mDNS responder started");
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);


  lastReadMillis = millis();
  lastMessageMillis = millis();

}

// main program
void loop() {
  server.handleClient();
  nowMillis = millis();

  if (nowMillis - lastReadMillis > dReadMillis) {
    Serial.println(timestamp() + "  time to get new values from sensor");
    metering();
  }

  if (!dMessageMillis == 0 && nowMillis - lastMessageMillis > dMessageMillis) {
    message_mqtt();

  }

  if (!dHumi == 0 && abs(humidity - humiditymqtt) >= dHumi) {
    Serial.println(timestamp() + "  send humidity value to mqtt");
    message_mqtt();

  }

  if (!dTemp == 0 && abs(temp - tempmqtt) >= dTemp) {
    Serial.println(timestamp() + "  send temperature value to mqtt");
    message_mqtt();

  }
}
