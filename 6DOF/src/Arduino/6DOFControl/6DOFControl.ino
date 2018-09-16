/*
                                      +-----+
         +----[PWR]-------------------| USB |--+
         |                            +-----+  |
         |         GND/RST2  [ ][ ]            |
         |       MOSI2/SCK2  [ ][ ]  A5/SCL[ ] |   C5
         |          5V/MISO2 [ ][ ]  A4/SDA[ ] |   C4
         |                             AREF[ ] |
         |                              GND[ ] |
         | [ ]N/C                    SCK/13[x] |   B5   SD SCK
         | [ ]IOREF                 MISO/12[x] |   .    SD MISO
         | [ ]RST                   MOSI/11[x]~|   .    SD MOSI
 SD  5V  | [x]3V3    +---+               10[x]~|   .    SD EN
 LCD 5V  | [x]5v    -| A |-               9[X]~|   .    AUDIO OUT L
 SD  GND | [x]GND   -| R |-               8[ ] |   B0
 LCD GND | [x]GND   -| D |-                    |
         | [ ]Vin   -| U |-               7[ ] |   D7
         |          -| I |-               6[ ]~|   .
         | [ ]A0    -| N |-               5[ ]~|   .
         | [ ]A1    -| O |-               4[ ] |   .
         | [ ]A2     +---+           INT1/3[ ]~|   .
         | [ ]A3                     INT0/2[ ] |   .
 LCD I2C | [x]A4/SDA  RST SCK MISO     TX>1[ ] |   .
 LCD I2C | [x]A5/SCL  [ ] [ ] [ ]      RX<0[ ] |   D0
         |            [ ] [ ] [ ]              |
         |  UNO_R3    GND MOSI 5V  ____________/
          \_______________________/

      http://busyducks.com/ascii-art-arduinos


                                      +-----+
         +----[PWR]-------------------| USB |--+
         |                            +-----+  |
         |           GND/RST2  [ ] [ ]         |
         |         MOSI2/SCK2  [ ] [ ]  SCL[ ] |   D0
         |            5V/MISO2 [ ] [ ]  SDA[ ] |   D1
         |                             AREF[ ] |
         |                              GND[ ] |
         | [ ]N/C                        13[ ]~|   B7
         | [ ]IOREF                      12[ ]~|   B6
         | [ ]RST                        11[ ]~|   B5
         | [ ]3V3      +----------+      10[ ]~|   B4
         | [ ]5v       | ARDUINO  |       9[ ]~|   H6
         | [ ]GND      |   MEGA   |       8[ ]~|   H5
         | [ ]GND      +----------+            |
         | [ ]Vin                         7[ ]~|   H4
         |                                6[ ]~|   H3
         | [ ]A0                          5[ ]~|   E3
         | [ ]A1                          4[ ]~|   G5
         | [ ]A2                     INT5/3[ ]~|   E5
         | [ ]A3                     INT4/2[ ]~|   E4
         | [ ]A4                       TX>1[ ]~|   E1
         | [ ]A5                       RX<0[ ]~|   E0
         | [ ]A6                               |
         | [ ]A7                     TX3/14[ ] |   J1
         |                           RX3/15[ ] |   J0
         | [ ]A8                     TX2/16[ ] |   H1
         | [ ]A9                     RX2/17[ ] |   H0
         | [ ]A10               TX1/INT3/18[ ] |   D3
         | [ ]A11               RX1/INT2/19[ ] |   D2
         | [ ]A12           I2C-SDA/INT1/20[ ] |   D1
         | [ ]A13           I2C-SCL/INT0/21[ ] |   D0
         | [ ]A14                              |
         | [ ]A15                              |   Ports:
         |                RST SCK MISO         |    22=A0  23=A1
         |         ICSP   [ ] [ ] [ ]          |    24=A2  25=A3
         |                [ ] [ ] [ ]          |    26=A4  27=A5
         |                GND MOSI 5V          |    28=A6  29=A7
         | G                                   |    30=C7  31=C6
         | N 5 5 4 4 4 4 4 3 3 3 3 3 2 2 2 2 5 |    32=C5  33=C4
         | D 2 0 8 6 4 2 0 8 6 4 2 0 8 6 4 2 V |    34=C3  35=C2
         |         ~ ~                         |    36=C1  37=C0
         | @ # # # # # # # # # # # # # # # # @ |    38=D7  39=G2
         | @ # # # # # # # # # # # # # # # # @ |    40=G1  41=G0
         |           ~                         |    42=L7  43=L6
         | G 5 5 4 4 4 4 4 3 3 3 3 3 2 2 2 2 5 |    44=L5  45=L4
         | N 3 1 9 7 5 3 1 9 7 5 3 1 9 7 5 3 V |    46=L3  47=L2
         | D                                   |    48=L1  49=L0    SPI:
         |                                     |    50=B3  51=B2     50=MISO 51=MOSI
         |     2560                ____________/    52=B1  53=B0     52=SCK  53=SS
          \_______________________/

         http://busyducks.com/ascii-art-arduinos
*/
/* Wants the following libraries:
 *  Adafruit-PWM-Servo-Driver-Library
 *  LiquidCrystal_I2C
 */
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_PWMServoDriver.h>
//#include <VarSpeedServo.h>

/* Servo control for AL5D arm */

void circle();
void line();
void zero_x();
void servo_park();
void set_arm( float x, float y, float z, float grip_angle_d, int servoSpeed );

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

/* Arm dimensions( mm ) */
#define BASE_HGT 73 //base hight 90
#define HUMERUS 98 //shoulder-to-elbow "bone" 100
#define ULNA 105 //elbow-to-wrist "bone" 135
#define GRIPPER 180 //gripper (incl.heavy duty wrist rotate mechanism) length 200

#define ftl(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5)) //float to long conversion
/* Servo names/numbers *
  Base servo HS-485HB */
#define BAS_SERVO 0
/* Shoulder Servo HS-5745-MG */
#define SHL_SERVO 1
/* Elbow Servo HS-5745-MG */
#define ELB_SERVO 2
/* Wrist servo HS-645MG */
#define WRI_SERVO 3
/* Wrist rotate servo HS-485HB */
#define WRO_SERVO 4
/* Gripper servo HS-422 */
#define GRI_SERVO 5

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);
// you can also call it with a different address and I2C interface
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(&Wire, 0x40);

// Depending on your servo make, the pulse width min and max may vary, you
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
#define SERVOMIN  150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // this is the 'maximum' pulse length count (out of 4096)



/* pre-calculations */

float hum_sq = HUMERUS * HUMERUS;
float uln_sq = ULNA * ULNA;
int servoSPeed = 10;
//ServoShield servos; //ServoShield object

//VarSpeedServo servo1, servo2, servo3, servo4, servo5, servo6;

int loopCounter = 0;
int pulseWidth = 6.6;
int microsecondsToDegrees;


long speed[] = {
  50, 100, 200, 250, 400, 500, 800
};
const int speeds = sizeof(speed) / sizeof(speed[0]);

// DELAY BETWEEN TESTS
#define RESTORE_LATENCY  5    // for delay between tests of found devices.
bool delayFlag = false;

// MINIMIZE OUTPUT
bool printAll = true;
bool header = true;

// STATE MACHINE
//enum states {
//  STOP, ONCE, CONT, HELP };
//states state = STOP;


uint32_t startScan;
uint32_t stopScan;
void ScanI2C(int sda, int scl)
{
  Wire.begin();

  // Source: http://forum.arduino.cc/index.php?topic=197360
  startScan = millis();
  uint8_t count = 0;

  if (header)
  {
    Serial.print(F("TIME\tDEC\tHEX\t"));
    for (uint8_t s = 0; s < speeds; s++)
    {
      Serial.print(F("\t"));
      Serial.print(speed[s]);
    }
    Serial.println(F("\t[KHz]"));
    for (uint8_t s = 0; s < speeds + 5; s++)
    {
      Serial.print(F("--------"));
    }
    Serial.println();
  }
  // TEST
  // 0.1.04: tests only address range 8..120
  // --------------------------------------------
  // Address  R/W Bit Description
  // 0000 000   0 General call address
  // 0000 000   1 START byte
  // 0000 001   X CBUS address
  // 0000 010   X reserved - different bus format
  // 0000 011   X reserved - future purposes
  // 0000 1XX   X High Speed master code
  // 1111 1XX   X reserved - future purposes
  // 1111 0XX   X 10-bit slave addressing
  for (uint8_t address = 8; address < 120; address++)
  {
    bool printLine = printAll;
    bool found[speeds];
    bool fnd = false;

    for (uint8_t s = 0; s < speeds ; s++)
    {
      TWBR = (F_CPU / (speed[s] * 1000) - 16) / 2;
      Wire.beginTransmission (address);
      found[s] = (Wire.endTransmission () == 0);
      fnd |= found[s];
      // give device 5 millis
      if (fnd && delayFlag) delay(RESTORE_LATENCY);
    }

    if (fnd) count++;
    printLine |= fnd;

    if (printLine)
    {
      Serial.print(millis());
      Serial.print(F("\t"));
      Serial.print(address, DEC);
      Serial.print(F("\t0x"));
      Serial.print(address, HEX);
      Serial.print(F("\t"));

      for (uint8_t s = 0; s < speeds ; s++)
      {
        Serial.print(F("\t"));
        Serial.print(found[s] ? F("V") : F("."));
      }
      Serial.println();
    }
  }

  stopScan = millis();
  if (header)
  {
    Serial.println();
    Serial.print(count);
    Serial.print(F(" devices found in "));
    Serial.print(stopScan - startScan);
    Serial.println(F(" milliseconds."));
  }
}



void servo(int servoNo, int microsecondsToDegrees, int servoSpeed)
{
  pwm.setPWM(servoNo, 0, microsecondsToDegrees);
}
void setup()
{
  int error;
  Serial.begin( 115200 );
  Serial.println("Start");
  //Wire.begin();
//  ScanI2C(4, 5);
  
//  lcd.init();                      // initialize the lcd 
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.home(); lcd.clear();
  lcd.print("6DOF - Ready");
  pwm.begin();
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

  delay(10);

  //  servo1.attach( BAS_SERVO, 544, 2400 );
  //  servo2.attach( SHL_SERVO, 544, 2400 );
  //  servo3.attach( ELB_SERVO, 544, 2400 );
  //  servo4.attach( WRI_SERVO, 544, 2400 );
  //  servo5.attach( WRO_SERVO, 544, 2400 );
  //  servo6.attach( GRI_SERVO, 544, 2400 );
  //delay( 5500 );
  //Serial.println("Servos attached");
  //servos.start(); //Start the servo shield
  servo_park();
//  delay(5000);
//  set_arm( -300, 0, 100, 0 , 10); //
}
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
void myprintln(String str)
{
  Serial.println(str);
  lcd.clear();
  lcd.print(str.substring(0,15));
  if(str.length()>15)
  {
    lcd.setCursor(0,1);
    lcd.print(str.substring(15));
  }
}
String command = "";
void loop()
{
  char c;
  if (Serial.available() > 0)
  {
    c = Serial.read();
    if (c == '\n' || c == '\r')
    {
      if (command.length() > 0)
      {
        command.trim();
        command.toUpperCase();
        // Echo command for port searching use.
        if (command.startsWith("6DOF?"))
        {
          myprintln("6DOF=Yes");
        } else if (command.startsWith("PARK"))
        {
          myprintln("Park");
          servo_park();
        } else if (command.startsWith("CIRCLE"))
        {
          myprintln("Circle");
          circle();
        } else if (command.startsWith("LINE"))
        {
          myprintln("Line");
          line();
        } else if (command.startsWith("ZERO_X"))
        {
          myprintln("Zero X");
          zero_x();
        }
        else if (command.startsWith("H"))
        {
          Serial.println("6DOF Code, I move a robot arm, tell me: [6DOF?, PARK, CIRCLE, LINE, ZERO_X, HELP, <x>,<y>,<z>,<grip angle>,<pinch>,<delay (ms)>]");
          lcd.clear();
          lcd.print("PARK,CIRCLE,LINE,ZERO_X");
          lcd.setCursor(0,1);
          lcd.print("x,y,z,ga,pch,dly");
        }
        else
        {
          // DONE: Parse command
          //set_arm( -300, 0, 100, 0 , 10); //
          float x, y, z, gripAngl, pinch;
          int sp;
          x = getValue(command, ',', 0).toFloat();
          y = getValue(command, ',', 1).toFloat();
          z = getValue(command, ',', 2).toFloat();
          gripAngl = getValue(command, ',', 3).toFloat();
          pinch = getValue(command, ',', 4).toFloat();
          sp = getValue(command, ',', 5).toInt();
          if (sp <= 0)
          {
            sp = 10;
          }
          
          myprintln((String)"M[" + String(x) + "," + String(y) + "," + String(z) + "]," + String(degrees(gripAngl)) + "," + String(degrees(pinch)) + ","+sp);          
          set_arm( x, y, z, gripAngl , sp);
          servo(GRI_SERVO, degrees(pinch), sp);
        }
      }
      command = "";
    }
    else
    {
      command += c;
    }
  }
  /*
    loopCounter += 1;
    //delay(7000);
    //delay(5000);
    //zero_x();
    //line();
    //circle();
    delay(4000);

    if (loopCounter > 1) {
    servo_park();
    //set_arm( 0, 0, 0, 0 ,10); // park
    delay(5000);
    exit(0);
    }//pause program - hit reset to continue
    //exit(0);
  */

}

/* arm positioning routine utilizing inverse kinematics *
  z is height, y is distance from base center out, x is side to side. y,z can only be positive
  //void set_arm( uint16_t x, uint16_t y, uint16_t z, uint16_t grip_angle )
*/
void set_arm( float x, float y, float z, float grip_angle_d, int servoSpeed )
{
  float grip_angle_r = radians( grip_angle_d ); //grip angle in radians for use in calculations

  /* Base angle and radial distance from x,y coordinates */
  float bas_angle_r = atan2( x, y );
  float rdist = sqrt(( x * x ) + ( y * y ));

  /* rdist is y coordinate for the arm */
  y = rdist;

  /* Grip offsets calculated based on grip angle */
  float grip_off_z = ( sin( grip_angle_r )) * GRIPPER;
  float grip_off_y = ( cos( grip_angle_r )) * GRIPPER;

  /* Wrist position */
  float wrist_z = ( z - grip_off_z ) - BASE_HGT;
  float wrist_y = y - grip_off_y;

  /* Shoulder to wrist distance ( AKA sw ) */
  float s_w = ( wrist_z * wrist_z ) + ( wrist_y * wrist_y );
  float s_w_sqrt = sqrt( s_w );

  /* s_w angle to ground */
  float a1 = atan2( wrist_z, wrist_y );

  /* s_w angle to humerus */
  float a2 = acos((( hum_sq - uln_sq ) + s_w ) / ( 2 * HUMERUS * s_w_sqrt ));

  /* shoulder angle */
  float shl_angle_r = a1 + a2;
  float shl_angle_d = degrees( shl_angle_r );

  /* elbow angle */
  float elb_angle_r = acos(( hum_sq + uln_sq - s_w ) / ( 2 * HUMERUS * ULNA ));
  float elb_angle_d = degrees( elb_angle_r );
  float elb_angle_dn = -( 180.0 - elb_angle_d );

  /* wrist angle */
  float wri_angle_d = ( grip_angle_d - elb_angle_dn ) - shl_angle_d;

  /* Servo pulses */
  float bas_servopulse = 1500.0 - (( degrees( bas_angle_r )) * pulseWidth );
  float shl_servopulse = 1500.0 + (( shl_angle_d - 90.0 ) * pulseWidth );
  float elb_servopulse = 1500.0 - (( elb_angle_d - 90.0 ) * pulseWidth );

  //float wri_servopulse = 1500 + ( wri_angle_d * pulseWidth );
  float wri_servopulse = 1500 - ( wri_angle_d * pulseWidth );// update 2018/2/11 by jimd - I think this should be minus, not plus
  /* Set servos */
  //servos.setposition( BAS_SERVO, ftl( bas_servopulse ));
  microsecondsToDegrees = map(ftl(bas_servopulse), 544, 2400, 0, 180);
  servo(BAS_SERVO, microsecondsToDegrees, servoSpeed); // use this function so that you can set servo speed //

  //servos.setposition( SHL_SERVO, ftl( shl_servopulse ));
  microsecondsToDegrees = map(ftl(shl_servopulse), 544, 2400, 0, 180);
  servo(SHL_SERVO, microsecondsToDegrees, servoSpeed);

  //servos.setposition( ELB_SERVO, ftl( elb_servopulse ));
  microsecondsToDegrees = map(ftl(elb_servopulse), 544, 2400, 0, 180);
  servo(ELB_SERVO, microsecondsToDegrees, servoSpeed);

  //servos.setposition( WRI_SERVO, ftl( wri_servopulse ));
  microsecondsToDegrees = map(ftl(wri_servopulse), 544, 2400, 0, 180);
  servo(WRI_SERVO, microsecondsToDegrees, servoSpeed);
}

/* move servos to parking position */
void servo_park()
{
  //servos.setposition( BAS_SERVO, 1500 );
  servo(BAS_SERVO, 90, 10);
  //servos.setposition( SHL_SERVO, 2100 );
  servo(SHL_SERVO, 90, 10);
  //servos.setposition( ELB_SERVO, 2100 );
  servo(ELB_SERVO, 90, 10);
  //servos.setposition( WRI_SERVO, 1800 );
  servo(WRI_SERVO, 90, 10);
  //servos.setposition( WRO_SERVO, 600 );
  servo(WRO_SERVO, 90, 10);
  //servos.setposition( GRI_SERVO, 900 );
  servo(GRI_SERVO, 80, 10);
  return;
}

void zero_x()
{
  for ( double yaxis = 250.0; yaxis < 400.0; yaxis += 1 ) {
    Serial.print(" yaxis= : "); Serial.println(yaxis);
    set_arm( 0, yaxis, 200.0, 0 , 10);
    delay( 10 );
  }

  for ( double yaxis = 400.0; yaxis > 250.0; yaxis -= 1 ) {
    set_arm( 0, yaxis, 200.0, 0 , 10);
    delay( 10 );
  }
}

/* moves arm in a straight line */
void line()
{
  for ( double xaxis = -100.0; xaxis < 100.0; xaxis += 0.5 ) {
    set_arm( xaxis, 250, 120, 0 , 10);
    delay( 10 );
  }

  for ( float xaxis = 100.0; xaxis > -100.0; xaxis -= 0.5 ) {
    set_arm( xaxis, 250, 120, 0 , 10);
    delay( 10 );
  }
}

void circle()
{
#define RADIUS 50.0
  //float angle = 0;
  float zaxis, yaxis;
  for ( float angle = 0.0; angle < 360.0; angle += 1.0 ) {
    yaxis = RADIUS * sin( radians( angle )) + 300;
    zaxis = RADIUS * cos( radians( angle )) + 200;
    set_arm( 0, yaxis, zaxis, 0 , 50);
    delay( 10 );
  }
}
