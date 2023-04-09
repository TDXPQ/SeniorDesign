/*
 * ESP32 AJAX Demo
 * Updates and Gets data from webpage without page refresh
 * https://circuits4you.com
 */
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <HardwareSerial.h>
#include <string.h>
#include <EEPROM.h>

#include "index.h"  //Web page header file
#include "config.h"
#include "credentials.h"

HardwareSerial SerialPort(1);
WebServer server(80);

//Enter your SSID and PASSWORD
const char* ssid = "MyAltice DE203F";
const char* password = "16-0205-violet";

String webpage = "main";

//Global String to display Data
char buffer[16];


//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
 String s;

 if(webpage.equals("main")){
   s = MAIN_page; //Read HTML contents
 } else if(webpage.equals("config")){
   s = CONFIG_page;
 } else if(webpage.equals("credentials")){
   s = CREDENTIAL_page;
 }
 
 server.send(200, "text/html", s); //Send web page
}

void handleConfigPage(){
  webpage = "config";
  handleRoot();
  server.send(200, "text/plain", "Config Page");
}

void handleRemotePage(){
  webpage = "main";
  handleRoot();
  server.send(200, "text/plain", "Remote Page");
}

void handleWifiCredPage(){
  webpage = "credentials";
  handleRoot();
  server.send(200, "text/plain", "Credentials Page");
}
 
void handlePackVoltage() {
  memset(buffer,'\0',16);
  SerialPort.print("a\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String pack_voltage_array = String(buffer);
 
 server.send(200, "text/plane", pack_voltage_array); //Send ADC value only to client ajax request
}

void handlePackCurrent() {
  memset(buffer,'\0',16);
  SerialPort.print("b\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String pack_current_array = String(buffer);
 
 server.send(200, "text/plane", pack_current_array); //Send ADC value only to client ajax request
}

void handlePackSOC() {
  memset(buffer,'\0',16);
  SerialPort.print("c\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String pack_soc_array = String(buffer);
 
 server.send(200, "text/plane", pack_soc_array); //Send ADC value only to client ajax request
}

void handlePackPower() {
  memset(buffer,'\0',16);
  SerialPort.print("d\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String pack_power_array = String(buffer);
 
 server.send(200, "text/plane", pack_power_array); //Send ADC value only to client ajax request
}

void handleVoltageValue1() {
  memset(buffer,'\0',16);
  SerialPort.print("e\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String voltageValue1 = String(buffer);
 
 server.send(200, "text/plane", voltageValue1); //Send ADC value only to client ajax request
}

void handleVoltageValue2() {
  memset(buffer,'\0',16);
  SerialPort.print("f\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String voltageValue2 = String(buffer);
 
 server.send(200, "text/plane", voltageValue2); //Send ADC value only to client ajax request
}

void handleVoltageValue3() {
  memset(buffer,'\0',16);
  SerialPort.print("g\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String voltageValue3 = String(buffer);
 
 server.send(200, "text/plane", voltageValue3); //Send ADC value only to client ajax request
}

void handleMotorValue() {
  memset(buffer,'\0',16);
  SerialPort.print("h\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String motorValue = String(buffer);
 
 server.send(200, "text/plane", motorValue); //Send ADC value only to client ajax request
}

void handleControllerValue() {
  memset(buffer,'\0',16);
  SerialPort.print("i\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String controllerValue = String(buffer);
 
 server.send(200, "text/plane", controllerValue); //Send ADC value only to client ajax request
}

void handleDCDCValue() {
  memset(buffer,'\0',16);
  SerialPort.print("j\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String dcdcValue = String(buffer);
 
 server.send(200, "text/plane", dcdcValue); //Send ADC value only to client ajax request
}

void handleBBoxValue1() {
  memset(buffer,'\0',16);
  SerialPort.print("k\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String bboxValue1 = String(buffer);
 
 server.send(200, "text/plane", bboxValue1); //Send ADC value only to client ajax request
}

void handleBBoxValue2() {
  memset(buffer,'\0',16);
  SerialPort.print("l\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String bboxValue2 = String(buffer);
 
 server.send(200, "text/plane", bboxValue2); //Send ADC value only to client ajax request
}

void handleAmbientValue() {
  memset(buffer,'\0',16);
  SerialPort.print("m\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String ambientValue = String(buffer);
 
 server.send(200, "text/plane", ambientValue); //Send ADC value only to client ajax request
}

void handleRestartServer() {
  SerialPort.print("RESTART\r\n");
}

void handleIpAddress() {
  String ipAddressValue = WiFi.localIP().toString();
 
  server.send(200, "text/plane", ipAddressValue); //Send ADC value only to client ajax request
}

void handleCurrentDNSValue() {
  String dnsValue = WiFi.dnsIP().toString();
 
  server.send(200, "text/plane", dnsValue); //Send ADC value only to client ajax request
}

void handleNetMaskValue() {
  String subNetMaskValue = WiFi.subnetMask().toString();
 
  server.send(200, "text/plane", subNetMaskValue); //Send ADC value only to client ajax request
}

void handleMACValue() {
  String macAddressValue = WiFi.macAddress();
 
  server.send(200, "text/plane", macAddressValue); //Send ADC value only to client ajax request
}

void storeStringToEEPROM(String str) {
  int len = str.length() + 1; // Include null character
  for (int i = 0; i < len; i++) {
    EEPROM.write(i, str.charAt(i));
  }
  EEPROM.commit();
}


String loadStringFromEEPROM() {
  String result;
  char c;
  int i = 0;
  do {
    c = EEPROM.read(i++);
    if (c != '\0') {
      result += c;
    }
  } while (c != '\0' && i < EEPROM.length());
  return result;
}

void handleSaveCreds() {
  //Serial.println("Save Button");
  String storeWifiCreds = server.arg("data"); // extract the data from the request body
  storeStringToEEPROM(storeWifiCreds);

  
  server.send(200, "text/plain", "Data received"); // send a response to the client
}

void handleLoadCreds(){
  String loadWifiCreds = loadStringFromEEPROM();
  //Serial.println(loadWifiCreds); // print the received data to the Serial Monitor

  server.send(200, "text/plain", loadWifiCreds); // send a response to the client
}

void connectWifi(){
  
  Serial.println("Reconnect Button Pressed");
  
  String loadWifiCreds = loadStringFromEEPROM();

  char* credsPointer = const_cast<char*>(loadWifiCreds.c_str());

  // Array to store the substrings
  char *credentials[6];

  // Initialize strtok with the string to be split and delimiter
  char *token = strtok(credsPointer, ",");

  for (int i = 0; i < 6; i++){
    credentials[i] = token;
    token = strtok(NULL, ",");
  }

  // Print out the resulting substrings
  for (int j = 0; j < 3; j++) {
    WiFi.begin(credentials[j], credentials[j+1]); // Connect to the Wi-Fi network using the corresponding SSID and password
    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi connection to be established
      delay(1000);
      Serial.print(".");
    }
  }



}


//===============================================================
// Setup
//===============================================================

void setup(void){
  EEPROM.begin(250);

  Serial.begin(115200);
  SerialPort.begin(115200, SERIAL_8N1, 40, 38);  
  Serial.println();
  Serial.println("Booting Sketch...");

/*
//ESP32 As access point
  WiFi.mode(WIFI_AP); //Access Point mode
  WiFi.softAP(ssid, password);
*/
//ESP32 connects to your wifi -----------------------------------
  WiFi.mode(WIFI_STA); //Connectto your wifi
  WiFi.begin(ssid, password);

  Serial.println("Connecting to ");
  Serial.print(ssid);

  //Wait for WiFi to connect
  while(WiFi.waitForConnectResult() != WL_CONNECTED){      
      Serial.print(".");
    }
    
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
//----------------------------------------------------------------
 
  server.on("/", handleRoot);      //This is display page
  server.on("/readPackVoltage", handlePackVoltage);//To get update of ADC Value only
  server.on("/readPackCurrent", handlePackCurrent);//To get update of ADC Value only
  server.on("/readSOCValue", handlePackSOC);//To get update of ADC Value only
  server.on("/readPowerValue", handlePackPower);//To get update of ADC Value only
  server.on("/readvoltageValue1", handleVoltageValue1);//To get update of ADC Value only
  server.on("/readvoltageValue2", handleVoltageValue2);//To get update of ADC Value only
  server.on("/readvoltageValue3", handleVoltageValue3);//To get update of ADC Value only

  server.on("/readMotorValue", handleMotorValue);//To get update of ADC Value only
  server.on("/readControllerValue", handleControllerValue);//To get update of ADC Value only
  server.on("/readDCDCValue", handleDCDCValue);//To get update of ADC Value only
  server.on("/readBBoxValue1", handleBBoxValue1);//To get update of ADC Value only
  server.on("/readBBoxValue2", handleBBoxValue2);//To get update of ADC Value only
  server.on("/readAmbientValue", handleAmbientValue);//To get update of ADC Value only

  server.on("/readIpAddressValue", handleIpAddress);//To get update of ADC Value only
  server.on("/currentDNSValue", handleCurrentDNSValue);//To get update of ADC Value only
  server.on("/readNetMaskValue", handleNetMaskValue);//To get update of ADC Value only
  server.on("/readMACValue", handleMACValue);//To get update of ADC Value only

  server.on("/readRestartServer", handleRestartServer);//To get update of ADC Value only

  server.on("/readConfigPage", handleConfigPage);//To get update of ADC Value only
  server.on("/readRemotePage", handleRemotePage);//To get update of ADC Value only
  server.on("/readWifiCredPage", handleWifiCredPage);//To get update of ADC Value only

  server.on("/saveWifiCreds", handleSaveCreds);//To get update of ADC Value only
  server.on("/readNetworkCreds", handleLoadCreds);//To get update of ADC Value only

  server.on("/readWifiReconnect", connectWifi);//To get update of ADC Value only
 
  server.begin();                  //Start server
  Serial.println("HTTP server started");

  
}

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void loop(void){
  server.handleClient();
  delay(1);

}
