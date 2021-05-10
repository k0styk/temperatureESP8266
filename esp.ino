//#define DEBUG_ENABLE
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <OneWire.h>
#include <DallasTemperature.h>
// контакт для передачи данных подключен к D1 на ESP8266 12-E (GPIO5):
#define ONE_WIRE_BUS 5
#ifndef STASSID
#define STASSID "ssid"
#define STAPSK  "password"
#endif

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

// fingerprint нужен для https соединения с удалённым сервером
//                                 F3    01    9C    00    C8    DD    DA    D0    2D    87    37    95    F5    B8    65    99    56    0D    C2    CE
const uint8_t fingerprint[20] = {0xF3, 0x01, 0x9C, 0x00, 0xC8, 0xDD, 0xDA, 0xD0, 0x2D, 0x87, 0x37, 0x95, 0xF5, 0xB8, 0x65, 0x99, 0x56, 0x0D, 0xC2, 0xCE};
const char* serverName = "https://3temp.kostyk.repl.co/setTemp"; // адрес сервера
const char* ssid     = STASSID;
const char* password = STAPSK;
unsigned long lastTime = 0; // переменные таймера
unsigned long timerDelay = 5000; // переменная задержки таймера

// инициализация переменных шины датчика и хранения инмормации
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress Thermometer;
ESP8266WiFiMulti wifiMulti;
String deviceAddressStr;
String temperatureStr;
int deviceCount = 0;

// функция опросника температуры у одного датчика
void getTemperature(int index) {
  float tempC;
  float tempF;
  do {
    sensors.requestTemperatures();
    tempC = sensors.getTempCByIndex(index);
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

  wifiMulti.addAP(ssid, password);

  DEBUGLN("Connecting ...");
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(1000);
    DEBUG('.');
  }
  DEBUGLN('\n');
  DEBUG("Connected to ");
  DEBUGLN(WiFi.SSID());              // Tell us what network we're connected to
  DEBUG("IP address:\t");
  DEBUGLN(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

  sensors.begin(); // опрос датчиков
  delay(500);
  DEBUGLN("Locating devices...");
  deviceCount = sensors.getDeviceCount(); // подсчёт количества датчиков на шине
  DEBUG("Found: ");
  DEBUGTYPE(deviceCount, DEC);
  DEBUGLN(" devices.");
  DEBUGLN("");
  
  DEBUGLN("Printing addresses:");
  for (int i = 0;  i < deviceCount;  i++)
  {
    sensors.getAddress(Thermometer, i); // получения адреса датчика
    setAddress(Thermometer);  // установка адресса датчика для вывода в сериальный порт
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
  if ((millis() - lastTime) > timerDelay) { // неблокируемый основной поток таймер
    if(WiFi.status()== WL_CONNECTED) { // проверка если подключены к wifi
      std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure); // создание переменной client
      client->setFingerprint(fingerprint); // устанавливаем ключ шифрования для соединения
      HTTPClient https; // создание переменной https
      if (https.begin(*client, serverName)) { // если соединение успешно
        for (int i = 0;  i < deviceCount;  i++) { // пробегаем по всем датчикам на шине
          sensors.getAddress(Thermometer, i); // получения адреса датчика
          setAddress(Thermometer); // установка адреса в переменную
          getTemperature(i); // установка температуры в переменную

          https.addHeader("Content-Type", "application/x-www-form-urlencoded"); // установка заголовков для отправки POST
          String httpRequestData = "name="+deviceAddressStr+"&t="+temperatureStr; // формирование POST строки
          int httpCode = https.POST(httpRequestData); // отправка и получение ответа

          if (httpCode > 0) { // если http < 0 то ошибка
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
              String payload = https.getString(); // извлекаем текст ответа, елси httpCode=200 то OK
              DEBUGLN(payload);
            }
          } else {
            DEBUG("[HTTPS] GET... failed, error: ");
            DEBUGLN(https.errorToString(httpCode).c_str());
          }
          https.end();
        }
      }
    }
    else {
      DEBUGLN("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
