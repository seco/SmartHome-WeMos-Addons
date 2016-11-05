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

//default custom static IP
char static_ip[16] = "192.168.1.40";
char static_gw[16] = "192.168.1.1";
char static_sn[16] = "255.255.255.0";
char hostname_sensor[40] = "hostname_sensor";
char ip_adress_ccu[40] = "192.168.1.98";
char variable_temp[40] = "standardroom_temperature";
char variable_humi[40] = "standardroom_humidity";
DHT dht(D4, DHT11);
