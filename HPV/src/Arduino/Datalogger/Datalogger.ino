/*
   PSHS Human Powered Vehicle
   Version: 1.0

   Description
   This is the code that records the Human Powered Vehicle data and stores it to local storage (SD) for later transmission to the remote server.
   Sensors include:
    1. Polar Heart Rate
    2. GPS Geoloaction data (Longitude, Latitude, Heading, Altitude)
    3. Date and Time from Real Time Clock (RTC)
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
   29/08/2018 V1.0  Initial code

   Authors:
   Initials Full name             Comment
   FH       Mr Fred Houweling
   [                         ]
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
  RFID_CS   -| A5/4        27/A10 |-  RXD
  SPI       -| SCK/5        33/A9 |-  TXD
  SPI       -| MOSI/18      15/A8 |-  RING
  SPI       -| MISO/19      32/A7 |-  DTR
  Serial1   -| RX/16        14/A6 |-  DHT 22 Data
  Serial1   -| TX/17       SCL/22 |-  I^2C Clock
  1Wire     -| 21          SDA/23 |-  I^2C Data
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
#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

const byte SD_chipSelect = 26;
const byte RFID_chipSelect = 4;
const byte HR_ISR_Pin = 34;
volatile int HRinterruptCounter = 0;
int HRnumberOfInterrupts = 0;

void IRAM_ATTR HRhandleInterrupt() {
  portENTER_CRITICAL_ISR(&mux);
  HRinterruptCounter++;
  portEXIT_CRITICAL_ISR(&mux);
}

void setupHeartRate()
{
  pinMode(HR_ISR_Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(HR_ISR_Pin), HRhandleInterrupt, FALLING);
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
  setupHeartRate();
}
void loop()
{
  String dataString = "HR,";
  while(HRinterruptCounter>0){

      portENTER_CRITICAL(&mux);
      HRinterruptCounter--;
      portEXIT_CRITICAL(&mux);

      HRnumberOfInterrupts++;
      //Serial.print("An interrupt has occurred. Total: ");
      //Serial.println(numberOfInterrupts);
  }
  dataString+=HRnumberOfInterrupts;
  HRnumberOfInterrupts = 0;
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }

}
