/*
   PSHS Aquatics Monitoring Code
   Version: 1.0
   Description
   This code is designed to run on the ESP32 Arduino compatible microcontroller with built in WiFi.
   This is a 3 volt device, it is not 5 volt tolerant.
   The program is designed to maintain a network connection to the database server to periodically upload it's sensor data.
   Sensors currently include:
   - OneWire dallas semiconductior temperature probes:
     On board
     In tank
   - DFRobot pH probe: https://www.dfrobot.com/product-1110.html
   - DHT22 based humidity and temperature sensor.
   - Water flow rate pulse counter, setup for interrupt based counting.
   - Received Signal Strength Indication (RSSI) for the WiFi connection.
   - Device serial number for data management (Based on the device MAC address)
   This data is assembled into a JSON data payload for transmission onto the server database for further storage and processing through HTTP based protocols.
   On connection failure the data payload is currently discarded and a new sampleing period begins.
   Reporting interval is currently once per second.
   
   History:
   Date       Ver   Comments
   ========== ===== ===================================================================================
   21/11/2017 V1.0  Clean up proof of concept code.

   Authors:
   Initials Full name             Comment
   FH       Mr Fred Houweling
   ZB       Zachary Bates         Water flow
   MV       Matthew Vella         Water folw, PH, trubidity.
   

   Sample serial log:
------------------------------
Connecting to domain: 192.168.1.4
POST /newdata.php HTTP/1.0
User-Agent: Arduino/1.0
Connection: close
Content-Length: 374
Content-Type: application/json

{
  "Location": "W1",
  "Id": "T0",
  "chipid": "240AC482EF240000",
  "now": 1438329,
  "LocalIP": "192.168.1.3",
  "RSSI": -72,
  "RSSIUnit": "dBm",
  "Flow": 0,
  "FlowUnit": "L/min",
  "FlowCalibration": 4.5,
  "phAvg": 4.65271,
  "phMin": 4.643311,
  "phMax": 4.663989,
  "phSamples": 2,
  "DH22Temp": 29.1,
  "DH22Hum": 56.79999,
  "DH22HIndex": 30.71517,
  "temp": [
    "0,1,2865652707000079,25.75",
    "1,1,28FF931C71150192,30.12"
  ],
  "tempUnit": "C"
}Connected!

------------------------------
HTTP/1.1 200 OK
Date: Mon, 20 Nov 2017 23:55:56 GMT
Server: Apache/2.4.10 (Debian)
Cache-Control: no-cache, must-revalidate
Pragma: no-cache
Content-Length: 18
Connection: close
Content-Type: text/html; charset=UTF-8

Got new data:<br>

closing connection

*/
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include <ArduinoJson.h>

// WiFi network name and password:
const char * networkName = "PSHS-EXTENSION-SCIENCE";
const char * networkPswd = "Password1";

// Location of Database Server Website:
//const char * hostDomain = "192.168.1.100";
const char * hostDomain = "192.168.1.4";
const int hostPort = 80;

// The array of DStemperature sensors are located on this pin.
const int ONE_WIRE_BUS = 18; // was 2
// Data to help with data management at the database end. 
const char * sensorLocation = "W1";     // Where is it?
const char * sensor_id = "AQMS2017-01"; // A meaningful identifier for the data.


#define DHTPIN 21     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value
float hic; // Heat Index

// Flow rate

const byte sensorPin       = 12;
//byte sensorInterrupt = digitalPinToInterrupt(sensorPin);  // 0 = digital pin 2

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;

volatile unsigned int pulseCount;

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;
// End flow rate

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const int BUTTON_PIN = 0;
const int LED_PIN = 2; //LED_BUILTIN; // 5;
int A0max;
int A0min;
unsigned long A0AccumulatedSamples;
long A0Samples;

/*
  Insterrupt Service Routine
*/
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
// pH sensing pin.
const int inPin = 33;

void ResetADC()
{
  pinMode(inPin, INPUT);
  A0Samples = 0;
  A0AccumulatedSamples = 0;
  A0min = A0max = SampleADC();
}
int SampleADC()
{
  int value;
  value = analogRead(inPin);

  A0AccumulatedSamples += value;
  A0Samples++;
  if (value > A0max)
  {
    A0max = value;
  }
  if (value < A0min)
  {
    A0min = value;
  }
  return value;
}
unsigned int A0getAvg()
{
  return (A0AccumulatedSamples / A0Samples);
}

float VoltageTopH(int voltage)
{
  const float ADCVoltage = 3.33;
  const float Offset = 0.0;
  return ( 3.5 * (voltage * ADCVoltage / 1024 / 6) + Offset);
}
void setup()
{
  // Initilize hardware:
  Serial.begin(115200);
  // Set up the status LED line as an output
  //pinMode(statusLed, OUTPUT);
  //digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  ResetADC();
  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  // The Hall-effect sensor is connected to pin 3 which uses interrupt defined by digitalPinToInterrupt().
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  pinMode(sensorPin, INPUT_PULLUP);
  //digitalWrite(sensorPin, HIGH);

  attachInterrupt(digitalPinToInterrupt(sensorPin), pulseCounter, FALLING);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  sensors.begin();
  dht.begin();

  digitalWrite(LED_PIN, HIGH); // LED off
  //Serial.print("Press button 0 to connect to ");
  Serial.println(hostDomain);
  // Make the sensors kick off a conversion cycle.
  sensors.requestTemperatures();
}

/**
   Main program loop
*/
boolean toggle = false;
void loop()
{
  SampleADC();
  //Serial.print("s");
  //delay(10);
  if ((millis() - oldTime) > 1000)   // Only process counters once per second
  {

    digitalWrite(LED_PIN, toggle ? LOW : HIGH); // Turn on LED
    // Start a conversion right now so it's ready for the next read. We will read other sensors while we wait.
    sensors.requestTemperatures();
    // Disable the interrupts to make sure the ISR can't fire overwriting the variable. This must be very fast as we can hurt other background tasks if we take too long.
    noInterrupts();
    unsigned int pulseCnt = pulseCount;
    pulseCount = 0;
    interrupts();

    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCnt) / calibrationFactor;
    // Reset the pulse counter so we can start incrementing again

    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();

    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;

    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;

    //unsigned int frac;

    // Print the flow rate for this second in litres / minute
    //Serial.print("Flow rate: ");
    //Serial.print(int(flowRate));  // Print the integer part of the variable
    //Serial.print("L/min");
    //Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    //Serial.print("Output Liquid Quantity: ");
    //Serial.print(totalMilliLitres);
    //Serial.println("mL");
    //Serial.print("\t");       // Print tab space
    //Serial.print(totalMilliLitres/1000);
    //Serial.print("L");
    hum = dht.readHumidity();
    temp = dht.readTemperature();
    hic = dht.computeHeatIndex(temp, hum, false);
    requestURL(hostDomain, hostPort); // Connect to server
    digitalWrite(LED_PIN, toggle ? HIGH : LOW); // Turn on LED
    toggle = toggle ? FALSE : TRUE;
    ResetADC();
  }
}

void connectToWiFi(const char * ssid, const char * pwd)
{
  int ledState = 0;

  printLine();
  Serial.println("Connecting to WiFi network: " + String(ssid));

  WiFi.begin(ssid, pwd);
  int timeout = 20;
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink LED while we're connecting:
    digitalWrite(LED_PIN, ledState);
    ledState = (ledState + 1) % 2; // Flip ledState
    delay(500);
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

void requestURL(const char * host, uint8_t port)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    // Connect to the WiFi network (see function below loop)
    connectToWiFi(networkName, networkPswd);
  }
  printLine();
  Serial.println("Connecting to domain: " + String(host));

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  // Prepare POST Data
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["Location"] = sensorLocation;
  root["Id"] = String(sensor_id);
  root["chipid"] = getSerial(); //ESP.getEfuseMac();
  root["now"] = millis();
  root["LocalIP"] = WiFi.localIP().toString();
  root["RSSI"] = WiFi.RSSI();
  root["RSSIUnit"] = "dBm";
  root["Flow"] = flowRate;
  root["FlowUnit"] = "L/min";
  root["FlowCalibration"] = calibrationFactor;
  root["phAvg"] = VoltageTopH(A0getAvg());
  root["phMin"] = VoltageTopH(A0min);
  root["phMax"] = VoltageTopH(A0max);
  root["phSamples"] = A0Samples;
  root["DH22Temp"] = temp;
  root["DH22Hum"] = hum;
  root["DH22HIndex"] = hic;
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
}

void printLine()
{
  Serial.println();
  for (int i = 0; i < 30; i++)
    Serial.print("-");
  Serial.println();
}

