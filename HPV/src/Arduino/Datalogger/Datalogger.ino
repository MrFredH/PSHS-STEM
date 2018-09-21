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

   Requires the following libraries:
   OneWire
   LiquidCrystal_I2C
   DallasTemperature.h>
   ArduinoJson.h>
   DHT.h

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
#ifndef ESP32
#pragma message(The data logger needs the Board to be set to the Adafruit ESP32 Feather, Please fix it in the Tools, Boards menu.)
#error Select ESP32 board.
#endif


//#include <TimeLib.h>

#include "Time.h"
//#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
//#include <TinyGPS.h>       // http://arduiniana.org/libraries/TinyGPS/
//#include <SoftwareSerial.h>

#ifdef WANT_WIFI
#include <WiFi.h>
#endif

//#include <WiFiEspClient.h>
//#include <WiFiEspServer.h>
//#include <WiFiEspUdp.h>

#include <Wire.h>
#include <SD.h>
#include <SPI.h>
//#include <sd_defines.h>
//#include <sd_diskio.h>
#include <OneWire.h>

#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>

#include <Adafruit_GPS.h>

// what's the name of the hardware serial port?
#define GPSSerial Serial1

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

#include <ArduinoJson.h>
//#include <DHT.h>
//#include "DHT.h"
#include "DHTesp.h"
//#include "Ticker.h"

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
// TODO: Read in the WiFi configurations and store them in memory.
// TODO: Need to support more than 1 access point, allow code to scan and connect to best one dynamically.
// WiFi network name and password:
char * networkName;
char * networkPswd;
char * secret;
// Location of Database Server Website:
char * hostDomain;
int hostPort;
// Name of this vehicle
char * vehicle;

char* ntpServer1 = "time.google.com";
char* ntpServer2 = "pool.ntp.org";
char* ntpServer3 = "time.nist.gov";
const long  gmtOffset_sec = 36000;
const int   daylightOffset_sec = 36000;

// The array of DStemperature sensors are located on this pin.
const int ONE_WIRE_BUS = 21;

#define DHTPIN 14     // what pin we're connected to
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino
DHTesp dht;
ComfortState cf;

int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value
float hic; // Heat Index
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
unsigned long oldTime;

void syncTimeSources()
{
  // TODO: Need to set localTime in the ESP on startup and whenever we see the need to correct.
  // Sources: RTC i2c module storing time during power off.
  //          NTP Servers read in from config file on SD or failover to defaults. Corrects RTC
  //          GPS -> Corrects local time, updates RTC
  // Try RTC, then attempt NTP update, GPS time is highest priority. NTP and GPS update RTC.
  // Trust GPS, then NTP, then RTC.
  // TODO: Store TZ data in SD config file.
  /*
    int epoch_time = 1527369964;
    timeval epoch = {epoch_time, 0};
    const timeval *tv = &epoch;
    timezone utc = {0,0};
    const timezone *tz = &utc;
    settimeofday(tv, tz)
  */
}

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
  GPS.begin(9600);
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz
     
  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);
  
  // Ask for firmware version
  GPSSerial.println(PMTK_Q_RELEASE);

  //setSyncProvider(RTC.get);   // the function to get the time from the RTC
  //if(timeStatus()!= timeSet)
  //{ // Failed RTC
  //}
  pinMode(SD_chipSelect, OUTPUT);
  syncTimeSources();
  Serial.print("Initializing SD card...");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SD_chipSelect, OUTPUT);
  // see if the card is present and can be initialized:
  if (!SD.begin(SD_chipSelect)) {
    Serial.println("Card failed, or not present, fatal.");
    // don't do anything more:
    //while (1)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);
      delay(250);
    }
  }
  Serial.println("card initialized.");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2, ntpServer3);
  setupISR();
  sensors.begin();
  //dht.begin();
  dht.setup(DHTPIN, DHTesp::DHT22);

  // Make the sensors kick off a conversion cycle.
  sensors.requestTemperatures();
  oldTime = millis();
}

#ifdef WANT_WIFI
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

#endif
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
unsigned long led_blink_hr;
unsigned long led_blink_hall;

void loop()
{
  if (RFIDinterruptCounter > 0) {

    portENTER_CRITICAL(&mux);
    RFIDinterruptCounter--;
    portEXIT_CRITICAL(&mux);

    RFIDnumberOfInterrupts++;
  }
  while (HRinterruptCounter > 0) {

    portENTER_CRITICAL(&mux);
    HRinterruptCounter--;
    portEXIT_CRITICAL(&mux);
    led_blink_hr = millis() + 300;
    HRnumberOfInterrupts++;
    //Serial.print("An interrupt has occurred. Total: ");
    //Serial.println(numberOfInterrupts);
  }
  while (HALLinterruptCounter > 0) {

    portENTER_CRITICAL(&mux);
    HALLinterruptCounter--;
    portEXIT_CRITICAL(&mux);
    led_blink_hall = millis() + 100;

    HALLnumberOfInterrupts++;
    //Serial.print("An interrupt has occurred. Total: ");
    //Serial.println(numberOfInterrupts);
  }
  if (led_blink_hr != 0 || led_blink_hall != 0)
  {
    if (led_blink_hr > millis() && led_blink_hall > millis())
    {
      digitalWrite(LED_BUILTIN, 0);
      led_blink_hr = 0;
      led_blink_hall = 0;
    }
    else
    {
      digitalWrite(LED_BUILTIN, 1);
    }
  }

  if ((millis() - oldTime) > 1000)   // Only process counters once per second
  {
    char c = GPS.read();
    if (GPS.newNMEAreceived()) {
      // a tricky thing here is if we print the NMEA sentence, or data
      // we end up not listening and catching other sentences!
      // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
      Serial.println(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
      if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
        return; // we can fail to parse a sentence in which case we should just wait for another
    }

#ifdef WANT_WIFI
    WiFiClient client;
#endif
    unsigned long resume_time = millis();
    sensors.requestTemperatures();

    StaticJsonDocument<800> doc;
    //JsonObject& root = doc.createObject();
    JsonObject root = doc.to<JsonObject>();
#ifdef WANT_WIFI
    if (WiFi.status() != WL_CONNECTED)
    {
      // Connect to the WiFi network (see function below loop)
      connectToWiFi(networkName, networkPswd);
    }
#endif
    //hum = dht.readHumidity();
    //temp = dht.readTemperature();
    //hic = dht.computeHeatIndex(dht.readTemperature(true), hum);
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
      root["time"] = (String)timeinfo.tm_mday + "/" + timeinfo.tm_mon + "/" + timeinfo.tm_year + " " + timeinfo.tm_hour + ":" + timeinfo.tm_min + ":" + timeinfo.tm_sec;
    }
    if (vehicle != NULL)
      root["vehicle"] = vehicle;
    root["chipid"] = getSerial();
    root["now"] = millis();
#ifdef WANT_WIFI
    root["LocalIP"] = WiFi.localIP().toString();
    root["RSSI"] = WiFi.RSSI();
    root["RSSIUnit"] = "dBm";
#endif
    root["GPSDate"] =  String(GPS.day) + "/" + String(GPS.month) + "/" + String(GPS.year);
    root["GPSTime"] = String(GPS.hour) + ":" + String(GPS.minute) + ":" + String(GPS.seconds) + "." + String(GPS.milliseconds);
    root["GPSFix"] = GPS.fix;
    root["GPSFixQ"] = GPS.fixquality;
    if (GPS.fix) {
      root["GPSLat"] = String(GPS.latitude) + ":" + String(GPS.lat);
      root["GPSLon"] = String(GPS.longitude) + ":" + String(GPS.lon);
      root["GPSSpd"] = GPS.speed;
      root["GPSAngle"] = GPS.angle;
      root["GPSAlt"] = GPS.altitude;
      root["GPSSats"] = GPS.satellites;
    }
    TempAndHumidity newValues = dht.getTempAndHumidity();
    // Check if any reads failed and exit early (to try again).
    if (dht.getStatus() != 0) {
      Serial.println("DHT22 error status: " + String(dht.getStatusString()));
    }
    else
    {
      root["DH22Temp"] = newValues.temperature;
      root["DH22Hum"] = newValues.humidity;
      root["DH22DewPoint"] = dht.computeDewPoint(newValues.temperature, newValues.humidity);
      root["DH22HIndex"] = dht.computeHeatIndex(newValues.temperature, newValues.humidity);
      // dht.getComfortRatio(cf, newValues.temperature, newValues.humidity);
      root["DH22ComfortRatio"] = dht.getComfortRatio(cf, newValues.temperature, newValues.humidity);
      root["DH22ComfortFactor"] = cf;
    }
    root["HeartRate"] = HRnumberOfInterrupts;
    root["HeartTime"] = millis() - oldTime;
    HRnumberOfInterrupts = 0;

    root["RevsRate"] = HALLnumberOfInterrupts;
    root["RevsTime"] = millis() - oldTime;
    HALLnumberOfInterrupts = 0;
    //    dataString+=HRnumberOfInterrupts;

    // Array of DS Temperature sensors, if any.
    JsonArray temps = root.createNestedArray("temp");

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
    serializeJsonPretty(root, out);
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("datalog.txt", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(out);
      dataFile.close();
      // print to the serial port too:
      Serial.println(out);
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening datalog.txt");
    }
    oldTime = resume_time;
    //root.prettyPrintTo(out);
    // POST Section
    String postRequest =
      (String)"POST /newdata.php HTTP/1.0\r\n" +
      "User-Agent: Arduino/1.0\r\n" +
      //"Host: " + server + "\r\n" +
      //"Accept: *" + "/" + "*\r\n" +
      "Connection: close\r\n" +
      "Content-Length: " + out.length() + "\r\n" +
      "Content-Type: application/json\r\n" +
      "\r\n" + out;

    // This will send the request to the server
    Serial.print(postRequest);
#ifdef WANT_WIFI

    if (!client.connect(hostDomain, hostPort))
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
#endif
  }
}
void printLine()
{
  Serial.println();
  for (int i = 0; i < 30; i++)
    Serial.print("-");
  Serial.println();
}
