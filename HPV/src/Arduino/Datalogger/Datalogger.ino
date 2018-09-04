/*
   PSHS Human Powered Vehicle
   Version: 1.0

   Description
   This is the code that records the Human Powered Vehicle data and stores it to local storage (SD) for later transmission to the remote server.
   Sensors include:
    1. Polar Heart Rate
    2. GPS Geoloaction data (Longitude, Latitude, Heading, Altitude)
    3. Date and Time from Real Time Clock (RTC)
    4. Temperature and Humidity (DHT22)
   Stage 1.
    Record Data and Append to local storage.
   Stage 2.
    Forward data to remote server.
   Stage 3.
    Develop a live feedback system for the records.

   History:
   Date       Ver   Comments
   ========== ===== ===================================================================================
   20/08/2018 V1.0  Code template.
   29/08/2018 V1.0  Initial code.
   04/09/2018 V1.0  First compilable version, feature poor.

   Authors:
   Initials Full name             Comment
   FH       Mr Fred Houweling
   [add your name above here, no surnames for under 18's]

             /====================\
             |        USB         |
            -| RST                |
            -| 3V        *1 LiPo- |-
            -| NC        *1 LiPo+ |-
            -| GND                |
  SD_CS     -| A0/DAC2/26  *2 BAT |-  4.2 Max, 3.7 Typ, 3.2 V Flat
  USB_VMON  -| A1/DAC1/25  *3  EN |-  Switch
  HR_ISR    -| A2/34          USB |-  Volt Meter Via Res Divider -> A1
  HALL_ISR  -| A3/39    LED13/A12 |-  LED
  RFID_ISR  -| A4/36   Boot12/A11 |-  n/c
  RFID_CS   -| A5/4        27/A10 |-  RXD  SIM800
  SPI       -| SCK/5        33/A9 |-  TXD  SIM800
  SPI       -| MOSI/18      15/A8 |-  RING SIM800
  SPI       -| MISO/19      32/A7 |-  DTR  SIM800
  GPSSerial1-| RX/16        14/A6 |-  DHT 22 Data
  GPSSerial1-| TX/17       SCL/22 |-  I^2C Clock : RTC, Baro, LCD
  1Wire     -| 21          SDA/23 |-  I^2C Data  : RTC, Baro, LCD
             |                    |
             |   ESP32 Feather    |
             |                    |
             \====================/

  1 Check polarity, Trace Neg to GND pin
  2 BAT – LiPo Battery
  3 Low signals 3.3V Reg Shutdown
*/
#include <Time.h>
//#include <TimeLib.h>
//#include <TinyGPS.h>       // http://arduiniana.org/libraries/TinyGPS/
//#include <SoftwareSerial.h>

#include <WiFi.h>
//#include <WiFiEspClient.h>
//#include <WiFiEspServer.h>
//#include <WiFiEspUdp.h>

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <sd_defines.h>
#include <sd_diskio.h>
#include <OneWire.h>
#include <LiquidCrystal_PCF8574.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
//#include <DHT.h>
#include "DHT.h"

// Define prototypes
void printLine();

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

// Mutually exclusive system to prevent deadlocks and races for resources. (ESP dual Processor issue)
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// Chip Select Pins
const byte SD_chipSelect = 26;
const byte RFID_chipSelect = 4;

// Interrupt constants
const byte HR_ISR_Pin = 34;
const byte HALL_ISR_Pin = 39;
const byte RFID_ISR_Pin = 36;

// Values that must remain fixed in memory as it's accessed by the interrupt service routines (ISR).
volatile int HRinterruptCounter = 0;
volatile int HALLinterruptCounter = 0;
volatile int RFIDinterruptCounter = 0;

int HRnumberOfInterrupts = 0;
int HALLnumberOfInterrupts = 0;
int RFIDnumberOfInterrupts = 0;

// NETWORK.ON.*.json
/*
   Look for files on the SD in this filename pattern NETWORK.ON.*.json
   in this format:
  {
  "Network": "ssidname",
  "NetworkPass": "ssidpass",
  "RemoteHost": "192.168.1.4",
  "RemotePort": 80,
  "secret": 1438329
  }
*/

// WiFi network name and password:
char * networkName;
char * networkPswd;
char * secret;
// Location of Database Server Website:
char * hostDomain;
int hostPort;
// Name of this vehicle
char * vehicle;

// The array of DStemperature sensors are located on this pin.
const int ONE_WIRE_BUS = 21;

#define DHTPIN 14     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino

int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value
float hic; // Heat Index
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
unsigned long oldTime;


void IRAM_ATTR HRhandleInterrupt() {
  portENTER_CRITICAL_ISR(&mux);
  HRinterruptCounter++;
  portEXIT_CRITICAL_ISR(&mux);
}

void IRAM_ATTR HALLhandleInterrupt() {
  portENTER_CRITICAL_ISR(&mux);
  HALLinterruptCounter++;
  portEXIT_CRITICAL_ISR(&mux);
}

void IRAM_ATTR RFIDhandleInterrupt() {
  portENTER_CRITICAL_ISR(&mux);
  RFIDinterruptCounter++;
  portEXIT_CRITICAL_ISR(&mux);
}

void setupISR()
{
  pinMode(HR_ISR_Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(HR_ISR_Pin), HRhandleInterrupt, FALLING);
  pinMode(HALL_ISR_Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(HALL_ISR_Pin), HALLhandleInterrupt, FALLING);
  pinMode(RFID_ISR_Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RFID_ISR_Pin), RFIDhandleInterrupt, FALLING);
}

void setup()
{
  Serial.begin(115200);
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(SD_chipSelect)) {
    Serial.println("Card failed, or not present, fatal.");
    // don't do anything more:
    while (1)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);
      delay(250);
    }
  }
  Serial.println("card initialized.");
  setupISR();
  sensors.begin();
  dht.begin();
  // Make the sensors kick off a conversion cycle.
  sensors.requestTemperatures();
  oldTime = millis();
}
void connectToWiFi(const char * ssid, const char * pwd)
{
  int ledState = 0;

  printLine();
  Serial.println("Connecting to WiFi network: " + String(ssid));

  WiFi.begin(ssid, pwd);
  int timeout = 2;
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink LED while we're connecting:
     digitalWrite(LED_BUILTIN, ledState);
    ledState = (ledState + 1) % 2; // Flip ledState
    delay(200);
    Serial.print(".");
    if (timeout == 0)
    {
      return;
      //esp_restart();
    }
    timeout--;
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

String DeviceAddresstoString(String& address, DeviceAddress deviceAddress)
{
  address = "";
  const char hex[] = "0123456789ABCDEF";
  for (uint8_t i = 0; i < 8; i++)
  {
    address += hex[deviceAddress[i] >> 4];
    address += hex[deviceAddress[i] & 0xf];
  }
  return (address);
}
// Get the device seria
String getSerial() {
  byte mac;
  uint64_t chipid;
  chipid = ESP.getEfuseMac();
  String cMac = "";
  for (int i = 0; i < 8; ++i)
  {
    mac = (chipid >> (8 * i)) & 0xff;
    if (mac < 0x10)
    {
      cMac += "0";
    }
    cMac += String(mac, HEX);
    //    if(i<5)
    //      cMac += ""; // put : or - if you want byte delimiters
  }
  cMac.toUpperCase();
  return cMac;
}

void loop()
{
  if (RFIDinterruptCounter > 0) {

    portENTER_CRITICAL(&mux);
    RFIDinterruptCounter--;
    portEXIT_CRITICAL(&mux);

    RFIDnumberOfInterrupts++;
  }

  if ((millis() - oldTime) > 1000)   // Only process counters once per second
  {
    unsigned long resume_time = millis();
    sensors.requestTemperatures();
    StaticJsonDocument<800> doc;
    //JsonObject& root = doc.createObject();
    JsonObject root = doc.to<JsonObject>();
    if (WiFi.status() != WL_CONNECTED)
    {
      // Connect to the WiFi network (see function below loop)
      connectToWiFi(networkName, networkPswd);
    }

    hum = dht.readHumidity();
    temp = dht.readTemperature();
    hic = dht.computeHeatIndex(temp, hum, false);
    root["vehicle"] = vehicle;
    root["chipid"] = getSerial();
    root["now"] = millis();
    root["LocalIP"] = WiFi.localIP().toString();
    root["RSSI"] = WiFi.RSSI();
    root["RSSIUnit"] = "dBm";
    // DH22
    root["DH22Temp"] = temp;
    root["DH22Hum"] = hum;
    root["DH22HIndex"] = hic;

    while (HRinterruptCounter > 0) {

      portENTER_CRITICAL(&mux);
      HRinterruptCounter--;
      portEXIT_CRITICAL(&mux);

      HRnumberOfInterrupts++;
      //Serial.print("An interrupt has occurred. Total: ");
      //Serial.println(numberOfInterrupts);
    }
    root["HeartRate"] = HRnumberOfInterrupts;
    root["HeartTime"] = millis() - oldTime;
    HRnumberOfInterrupts=0;
    
    while (HALLinterruptCounter > 0) {

      portENTER_CRITICAL(&mux);
      HALLinterruptCounter--;
      portEXIT_CRITICAL(&mux);

      HALLnumberOfInterrupts++;
      //Serial.print("An interrupt has occurred. Total: ");
      //Serial.println(numberOfInterrupts);
    }
    root["RevsRate"] = HALLnumberOfInterrupts;
    root["RevsTime"] = millis() - oldTime;
    HALLnumberOfInterrupts=0;
    //    dataString+=HRnumberOfInterrupts;

    // Array of DS Temperature sensors, if any.
    JsonArray& temps = root.createNestedArray("temp");
    for (int i = 0 ; i < sensors.getDeviceCount(); i++)
    {
      DeviceAddress deviceAddress;
      String address = "";
      bool attached = sensors.getAddress(deviceAddress, i);
      temps.add((String)i + ","
                + attached + ","
                + DeviceAddresstoString(address, deviceAddress) + ","
                + sensors.getTempCByIndex(i));
    }
    root["tempUnit"] = "C";
    HRnumberOfInterrupts = 0;
    HALLnumberOfInterrupts = 0;
    String out;
    root.prettyPrintTo(out);
    // POST Section
    String postRequest =
      (String)"POST /newdata.php HTTP/1.0\r\n" +
      "User-Agent: Arduino/1.0\r\n" +
      //"Host: " + server + "\r\n" +
      //"Accept: *" + "/" + "*\r\n" +
      "Connection: close\r\n" +
      "Content-Length: " + root.measureLength() + "\r\n" +
      "Content-Type: application/json\r\n" +
      "\r\n" + out;

    // This will send the request to the server
    Serial.print(postRequest);
    if (!client.connect(host, port))
    {
      Serial.println("connection failed");
      return;
    }
    Serial.println("Connected!");
    printLine();
    client.print(postRequest);
    unsigned long timeout = millis();
    while (client.available() == 0)
    {
      if (millis() - timeout > 5000)
      {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    // Read all the lines of the reply from server and print them to Serial
    while (client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

    Serial.println();
    Serial.println("closing connection");
    client.stop();
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("datalog.txt", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(out);
      dataFile.close();
      // print to the serial port too:
      Serial.println(dataString);
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening datalog.txt");
    }
    oldTime = resume_time;
  }
}
void printLine()
{
  Serial.println();
  for (int i = 0; i < 30; i++)
    Serial.print("-");
  Serial.println();
}
