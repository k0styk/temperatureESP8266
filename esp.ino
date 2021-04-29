#define DEBUG_ENABLE
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <OneWire.h>
#include <DallasTemperature.h>
// контакт для передачи данных подключен к D1 на ESP8266 12-E (GPIO5):
#define ONE_WIRE_BUS 5

#ifdef DEBUG_ENABLE
  #define DEBUG(x) Serial.print(x)
#else
  #define DEBUG(x)
#endif

#ifdef DEBUG_ENABLE
  #define DEBUGLN(x) Serial.println(x)
#else
  #define DEBUGLN(x)
#endif

#ifdef DEBUG_ENABLE
#define DEBUGTYPE(x,t) Serial.println(x,t)
#else
#define DEBUGTYPE(x,t)
#endif

/* Set these to your desired credentials. */
const char *ssid = "";
const char *password = "";
const char *host = "";

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature DS18B20(&oneWire);
DeviceAddress Thermometer;
String deviceAddressStr;
String temperatureStr;
// char temperatureCString[6];
// char temperatureFString[6];
int deviceCount = 0;

//=======================================================================
//                    Power on setup
//=======================================================================

void getTemperature() {
  float tempC;
  float tempF;
  do {
    DS18B20.requestTemperatures();
    tempC = DS18B20.getTempCByIndex(0);
    temperatureStr = String(tempC,2);
    // dtostrf(tempC, 2, 2, temperatureCString);
    delay(100);
  } while (tempC == 85.0 || tempC == (-127.0));
}

void setAddress(DeviceAddress deviceAddress)
{ 
  for (uint8_t i = 0; i < 8; i++)
  {
    deviceAddressStr = "0x";
    DEBUG("0x");
    if (deviceAddress[i] < 0x10) {
      DEBUG("0");
      deviceAddressStr += "0";
    }
    deviceAddressStr += String(deviceAddress[i], HEX); 
    DEBUGTYPE(deviceAddress[i], HEX);
    if (i < 7) DEBUG(", ");
  }
  DEBUGLN("");
}

void setup() {
  DS18B20.begin();

  delay(1000);
  #ifdef DEBUG_ENABLE // так же зависит от первоначальной переменной
    Serial.begin(115200);
  #endif
  // WiFiManager
  WiFiManager wifiManager;

  //wifiManager.resetSettings(); // run it once, if you want to erase all the stored information
  wifiManager.setAPConfig(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(255,255,255,0));

  wifiManager.autoConnect("ESP_WiFi");
  //wifiManager.autoConnect(); // or use this for auto generated name ESP + ChipID
  
  // if you get here you have connected to the WiFi
  DEBUGLN("Connected.");
  // WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  // delay(1000);
  // WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  
  // WiFi.begin(ssid, password);     //Connect to your WiFi router
  DEBUGLN("Connecting");
  // Wait for connection
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   DEBUG(".");
  // }

  // //If connection successful show IP address in serial monitor
  // delay(1000);
  // DEBUGLN("");
  // DEBUGLN("Connected to ");
  // DEBUGLN(ssid);
  // DEBUGLN("IP address: ");
  // DEBUGLN(WiFi.localIP());  //IP address assigned to your ESP
  
  DEBUGLN("Locating devices...");
  DEBUG("Found ");
  deviceCount = sensors.getDeviceCount();
  DEBUGTYPE(deviceCount, DEC);
  DEBUGLN(" devices.");
  DEBUGLN("");
  
  DEBUGLN("Printing addresses...");
  for (int i = 0;  i < deviceCount;  i++)
  {
    DEBUG("Sensor ");
    DEBUG(i+1);
    DEBUG(" : ");
    sensors.getAddress(Thermometer, i);
    setAddress(Thermometer);
  }
}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  HTTPClient http;    //Declare object of class HTTPClient

  getTemperature();
  // client.println(temperatureCString);
  // client.println(temperatureFString);

  //Post Data
  postData = "name="+deviceAddressStr+"&t="+temperatureStr;
  
  http.begin("https://3temp.kostyk.repl.co/setTemp");              //Specify request destination
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header

  int httpCode = http.POST(postData);   //Send the request
  String payload = http.getString();    //Get the response payload

  DEBUG(httpCode);   //Print HTTP return code
  DEBUG(payload);    //Print request response payload

  http.end();  //Close connection
  
  delay(5000);  //Post Data at every 5 seconds
}