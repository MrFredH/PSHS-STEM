//#include <VarSpeedServo.h>

/* Servo control for AL5D arm */

void circle();
void line();
void zero_x();
void servo_park();
void set_arm( float x, float y, float z, float grip_angle_d, int servoSpeed );

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

/* pre-calculations */

float hum_sq = HUMERUS * HUMERUS;
float uln_sq = ULNA * ULNA;
int servoSPeed = 10;
//ServoShield servos; //ServoShield object

VarSpeedServo servo1, servo2, servo3, servo4, servo5, servo6;

int loopCounter = 0;
int pulseWidth = 6.6;
int microsecondsToDegrees;

void setup()
{
  Serial.begin( 9600 );
  Serial.println("Start");
  servo1.attach( BAS_SERVO, 544, 2400 );
  servo2.attach( SHL_SERVO, 544, 2400 );
  servo3.attach( ELB_SERVO, 544, 2400 );
  servo4.attach( WRI_SERVO, 544, 2400 );
  servo5.attach( WRO_SERVO, 544, 2400 );
  servo6.attach( GRI_SERVO, 544, 2400 );
  //delay( 5500 );
  Serial.println("Servos attached");
  //servos.start(); //Start the servo shield
  servo_park();
  set_arm( -300, 0, 100, 0 ,10); //
}

void loop()
{
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
  servo1.write(microsecondsToDegrees, servoSpeed); // use this function so that you can set servo speed //
  
  //servos.setposition( SHL_SERVO, ftl( shl_servopulse ));
  microsecondsToDegrees = map(ftl(shl_servopulse), 544, 2400, 0, 180);
  servo2.write(microsecondsToDegrees, servoSpeed);
  
  //servos.setposition( ELB_SERVO, ftl( elb_servopulse ));
  microsecondsToDegrees = map(ftl(elb_servopulse), 544, 2400, 0, 180);
  servo3.write(microsecondsToDegrees, servoSpeed);

  //servos.setposition( WRI_SERVO, ftl( wri_servopulse ));
  microsecondsToDegrees = map(ftl(wri_servopulse), 544, 2400, 0, 180);
  servo4.write(microsecondsToDegrees, servoSpeed);
}

/* move servos to parking position */
void servo_park()
{
  //servos.setposition( BAS_SERVO, 1500 );
  servo1.write(90, 10);
  //servos.setposition( SHL_SERVO, 2100 );
  servo2.write(90, 10);
  //servos.setposition( ELB_SERVO, 2100 );
  servo3.write(90, 10);
  //servos.setposition( WRI_SERVO, 1800 );
  servo4.write(90, 10);
  //servos.setposition( WRO_SERVO, 600 );
  servo5.write(90, 10);
  //servos.setposition( GRI_SERVO, 900 );
  servo6.write(80, 10);
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
