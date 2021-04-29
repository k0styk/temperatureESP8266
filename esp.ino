#define DEBUG_ENABLE
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
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
#define DEBUGTYPE(x,t) Serial.print(x,t)
#else
#define DEBUGTYPE(x,t)
#endif

//                                 F3    01    9C    00    C8    DD    DA    D0    2D    87    37    95    F5    B8    65    99    56    0D    C2    CE
const uint8_t fingerprint[20] = {0xF3, 0x01, 0x9C, 0x00, 0xC8, 0xDD, 0xDA, 0xD0, 0x2D, 0x87, 0x37, 0x95, 0xF5, 0xB8, 0x65, 0x99, 0x56, 0x0D, 0xC2, 0xCE};
const char* serverName = "https://3temp.kostyk.repl.co/setTemp";
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);
DeviceAddress Thermometer;
String deviceAddressStr;
String temperatureStr;
int deviceCount = 0;

void getTemperature() {
  float tempC;
  float tempF;
  do {
    sensors.requestTemperatures();
    tempC = sensors.getTempCByIndex(0);
    temperatureStr = String(tempC,2);
    delay(100);
  } while (tempC == 85.0 || tempC == (-127.0));
}

void setAddress(DeviceAddress deviceAddress)
{ 
  deviceAddressStr = "0x";
  for (uint8_t i = 0; i < 8; i++)
  {    
    if (deviceAddress[i] < 0x10) { deviceAddressStr += "0"; }
    deviceAddressStr += String(deviceAddress[i], HEX);
  }
}

//=======================================================================
//                    Power on setup
//=======================================================================
void setup() {
  #ifdef DEBUG_ENABLE // так же зависит от первоначальной переменной
    Serial.begin(115200);
  #endif
  // WiFiManager
  WiFiManager wifiManager;
//  wifiManager.resetSettings(); // run it once, if you want to erase all the stored information
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.autoConnect("ESP_WiFi");
  DEBUGLN("Connected.");  

  sensors.begin();
  delay(500);
  DEBUGLN("Locating devices...");
  deviceCount = sensors.getDeviceCount();
  DEBUG("Found: ");
  DEBUGTYPE(deviceCount, DEC);
  DEBUGLN(" devices.");
  DEBUGLN("");
  
  DEBUGLN("Printing addresses:");
  for (int i = 0;  i < deviceCount;  i++)
  {
    sensors.getAddress(Thermometer, i);
    setAddress(Thermometer);
    DEBUG("Sensor ");
    DEBUG(i+1);
    DEBUG(" : ");
    DEBUGLN(deviceAddressStr);
  }
}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  if ((millis() - lastTime) > timerDelay) {
    if(WiFi.status()== WL_CONNECTED){
      std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
      client->setFingerprint(fingerprint);
      HTTPClient https;
      getTemperature();

      if (https.begin(*client, serverName)) {  // HTTPS
        https.addHeader("Content-Type", "application/x-www-form-urlencoded");
        String httpRequestData = "name="+deviceAddressStr+"&t="+temperatureStr;
        int httpCode = https.POST(httpRequestData);

        if (httpCode > 0) {
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
            DEBUGLN(payload);
          }
        } else {
          DEBUG("[HTTPS] GET... failed, error: ");
          DEBUGLN(https.errorToString(httpCode).c_str());
        }
        https.end();
      }
    }
    else {
      DEBUGLN("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
