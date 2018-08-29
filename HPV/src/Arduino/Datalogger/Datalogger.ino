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
SD_CS     -| A0/DAC2     *2 BAT |-  4.2 Max, 3.7 Typ, 3.2 V Flat
USB_VMON  -| A1/DAC1     *3  EN |-  Switch
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
  
*1 Check polarity, Trace Neg to GND pin
*2 BAT – LiPo Battery
*3 Low signals 3.3V Reg Shutdown

 */
 
