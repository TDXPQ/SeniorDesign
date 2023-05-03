/*
 * ESP32 AJAX Demo
 * Updates and Gets data from webpage without page refresh
 */


// ===Global Libraries===
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <HardwareSerial.h>
#include <string.h>
#include <EEPROM.h>

// ===Web page header files===
#include "index.h"
#include "config.h"
#include "credentials.h"

//===Global Variables=== 

// Set up ports and AJAX server
HardwareSerial SerialPort(1);
WebServer server(80);

IPAddress local_ip(192, 168, 1, 1); // The desired IP address for the access point
IPAddress gateway(192, 168, 1, 1); // The IP address of the gateway (the access point itself)
IPAddress subnet(255, 255, 255, 0); // The subnet mask for the network

// Global Strings
String webpage = "main";
char buffer[16];


//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {   // Webpage global variable choices which page to load 
 String s;

  //If condition to choice which page to load
  if(webpage.equals("main")){
    s = MAIN_page; //Read HTML contents
  } else if(webpage.equals("config")){
    s = CONFIG_page;
  } else if(webpage.equals("credentials")){
    s = CREDENTIAL_page;
  }
 
  server.send(200, "text/html", s); //Send web page
}

// ===Functions to handle HTML Page change requests===

void handleConfigPage(){
  webpage = "config"; // Change global webpage variable
  handleRoot(); // Call handleRoot() function to change page
  server.send(200, "text/plain", "Config Page"); // Send back message to the client side
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
 
// ===Functions to request information and actions from the AVR128===
void handlePackVoltage() {
  memset(buffer,'\0',16); // Reset the global buffer variable
  SerialPort.print("a\r\n"); // Send serial usart command to request information
  SerialPort.readBytesUntil('\n', buffer, 16); // Wait for the information to return from the AVR128
  String pack_voltage_array = String(buffer); // Store the buffer information to a variable
 
 server.send(200, "text/plane", pack_voltage_array); // Send information to client ajax request
}

//The following functtions are similiar to the first

void handlePackCurrent() {
  memset(buffer,'\0',16);
  SerialPort.print("b\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String pack_current_array = String(buffer);
 
 server.send(200, "text/plane", pack_current_array);
}

void handlePackSOC() {
  memset(buffer,'\0',16);
  SerialPort.print("c\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String pack_soc_array = String(buffer);
 
 server.send(200, "text/plane", pack_soc_array);
}

void handlePackPower() {
  memset(buffer,'\0',16);
  SerialPort.print("d\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String pack_power_array = String(buffer);
 
 server.send(200, "text/plane", pack_power_array); 
}

void handleVoltageValue1() {
  memset(buffer,'\0',16);
  SerialPort.print("e\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String voltageValue1 = String(buffer);
 
 server.send(200, "text/plane", voltageValue1);
}

void handleVoltageValue2() {
  memset(buffer,'\0',16);
  SerialPort.print("f\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String voltageValue2 = String(buffer);
 
 server.send(200, "text/plane", voltageValue2);
}

void handleVoltageValue3() {
  memset(buffer,'\0',16);
  SerialPort.print("g\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String voltageValue3 = String(buffer);
 
 server.send(200, "text/plane", voltageValue3);
}

void handleMotorValue() {
  memset(buffer,'\0',16);
  SerialPort.print("h\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String motorValue = String(buffer);
 
 server.send(200, "text/plane", motorValue);
}

void handleControllerValue() {
  memset(buffer,'\0',16);
  SerialPort.print("i\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String controllerValue = String(buffer);
 
 server.send(200, "text/plane", controllerValue);
}

void handleDCDCValue() {
  memset(buffer,'\0',16);
  SerialPort.print("j\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String dcdcValue = String(buffer);
 
 server.send(200, "text/plane", dcdcValue);
}

void handleBBoxValue1() {
  memset(buffer,'\0',16);
  SerialPort.print("k\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String bboxValue1 = String(buffer);
 
 server.send(200, "text/plane", bboxValue1);
}

void handleBBoxValue2() {
  memset(buffer,'\0',16);
  SerialPort.print("l\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String bboxValue2 = String(buffer);
 
 server.send(200, "text/plane", bboxValue2);
}

void handleAmbientValue() {
  memset(buffer,'\0',16);
  SerialPort.print("m\r\n");
  SerialPort.readBytesUntil('\n', buffer, 16);
  String ambientValue = String(buffer);
 
 server.send(200, "text/plane", ambientValue);
}

void handleRestartServer() {
  SerialPort.print("RESTART\r\n");
}

// ===Functions to request information from the ESP32===
void handleIpAddress() {
  String ipAddressValue = WiFi.localIP().toString(); // Get required information
  server.send(200, "text/plane", ipAddressValue);  // Send information to client side via HTTP request
}

// Following functions are similiar to the first function

void handleCurrentDNSValue() {
  String dnsValue = WiFi.dnsIP().toString();
  server.send(200, "text/plane", dnsValue);
}

void handleNetMaskValue() {
  String subNetMaskValue = WiFi.subnetMask().toString();
  server.send(200, "text/plane", subNetMaskValue);
}

void handleMACValue() {
  String macAddressValue = WiFi.macAddress();
  server.send(200, "text/plane", macAddressValue);
}

// ===Following funtions are used to store WiFi Credentials in the EEPROM===

void storeStringToEEPROM(String str) {
  int len = str.length() + 1; // Include null character
  for (int i = 0; i < len; i++) { // Write each character of the string to the EEPROM
    EEPROM.write(i, str.charAt(i));
  }
  EEPROM.commit(); // Commit the EEPROM changes
}


String loadStringFromEEPROM() {
  String result;
  char c;
  int i = 0;
  do { // Read each character from the EEPROM
    c = EEPROM.read(i++);
    if (c != '\0') {
      result += c;
    }
  } while (c != '\0' && i < EEPROM.length());
  return result; // Return the string result from the EEPROM
}

// Function to handle HTTPS Request to save new WiFi credentials
void handleSaveCreds() {
  //Serial.println("Save Button");
  String storeWifiCreds = server.arg("data"); // extract the data from the request body
  storeStringToEEPROM(storeWifiCreds); // Store the new credentials in the EEPROM
  
  server.send(200, "text/plain", "Data received"); // send a acknowldgment response to the client
}

// Function to handle sending the WiFi credentials to the client
void handleLoadCreds(){
  String loadWifiCreds = loadStringFromEEPROM(); // Load the data from them EEPROM to a string
  //Serial.println(loadWifiCreds); // print the received data to the Serial Monitor

  server.send(200, "text/plain", loadWifiCreds); // Send the WiFi credential data to the client side.
}

// ===Function to use EEPROM credentials to connect to WiFi===
void connectWifi(){
  
  //Serial.println("Reconnect Button Pressed");
  
  String loadWifiCreds = loadStringFromEEPROM(); // Load the EEPROM data to a string
  char* credsPointer = const_cast<char*>(loadWifiCreds.c_str()); // Cast the string to a character pointer to be used by the strtok function

  char *credentials[6]; // Character array to store 3 network name and password pairs
  char *token = strtok(credsPointer, ","); // Use strtok to seperate the wifi credentials by commans (the credentials are stored as commma seperated list)

  for (int i = 0; i < 6; i++){
    credentials[i] = token;  // Store each substring in the character array
    token = strtok(NULL, ","); // Retrieve the next substring
  }

   // Try each of the WiFi credentials
  for (int j = 0; j < 3; j++) {
    WiFi.begin(credentials[j], credentials[j+1]); // Try the network name and password pair
    delay(2000); // Provide a delay to see if the ESP32 connects to the the network
    //WiFi.config(local_ip, gateway, subnet); Commented out untill we can set up a static IP with the router
    if (WiFi.isConnected()) { // If the WiFi is connected do the following
      WiFi.softAPdisconnect(true); // Disable the ESP32 Access Point mode
      Serial.println("SoftAP disabled");
      break; // break to not try the other WiFi credentials
    }
  }
}


//===============================================================
// Setup
//===============================================================

void setup(void){
  EEPROM.begin(250); // Establish the needed EEPROM size

  Serial.begin(115200); // Define the serial BAUD Rate
  SerialPort.begin(115200, SERIAL_8N1, 40, 38);  // Establish the BAUD Rate, Standard, and GPIO pins
  Serial.println();
  Serial.println("Booting Sketch...");


  //ESP32 connects to your wifi -----------------------------------
  WiFi.mode(WIFI_STA); //Establish WiFi mode
  connectWifi(); // Try connecting to the saved WiFi Credentials

  // If connection failed, switch to AP mode
  if (WiFi.status() != WL_CONNECTED) {
    webpage = "config"; // Change the webpage default to the config pagge
    Serial.println("Failed to connect to WiFi network!");
    WiFi.disconnect(); // Disconnect WiFi
    WiFi.softAP("MyESP32AP", "MyAPPassword"); // Establish Access Point with ssid and password needed to connect
    WiFi.softAPConfig(local_ip, gateway, subnet); // Establish IP configuration
    Serial.println("Access point mode activated!");
    Serial.print("Connect to: ");
    Serial.println(WiFi.softAPIP());
  } else{
    //If connection successful show IP address in serial monitor
    
    Serial.println("");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  }

  
    
  
//----------------------------------------------------------------
 

  // HTTPS Request Function Handlers
  server.on("/", handleRoot);      //This is display page

  server.on("/readPackVoltage", handlePackVoltage);
  server.on("/readPackCurrent", handlePackCurrent);
  server.on("/readSOCValue", handlePackSOC);
  server.on("/readPowerValue", handlePackPower);
  server.on("/readvoltageValue1", handleVoltageValue1);
  server.on("/readvoltageValue2", handleVoltageValue2);
  server.on("/readvoltageValue3", handleVoltageValue3);

  server.on("/readMotorValue", handleMotorValue);
  server.on("/readControllerValue", handleControllerValue);
  server.on("/readDCDCValue", handleDCDCValue);
  server.on("/readBBoxValue1", handleBBoxValue1);
  server.on("/readBBoxValue2", handleBBoxValue2);
  server.on("/readAmbientValue", handleAmbientValue);

  server.on("/readIpAddressValue", handleIpAddress);
  server.on("/readDNSValue", handleCurrentDNSValue);
  server.on("/readNetMaskValue", handleNetMaskValue);
  server.on("/readMACValue", handleMACValue);

  server.on("/readRestartServer", handleRestartServer);

  server.on("/readConfigPage", handleConfigPage);
  server.on("/readRemotePage", handleRemotePage);
  server.on("/readWifiCredPage", handleWifiCredPage);

  server.on("/saveWifiCreds", handleSaveCreds);
  server.on("/readNetworkCreds", handleLoadCreds);

  server.on("/readWifiReconnect", connectWifi);
 
  server.begin();                  //Start server
  Serial.println("HTTP server started");

  
}

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void loop(void){
  server.handleClient(); // Hangle incoming client requests
  delay(1);
}
