//--------------------------------------------------------------------------
// Morse Code
// Get it? Morse code? Because it's code for doing Morse code!
//--------------------------------------------------------------------------

// Includes
#include <math.h>

// Pin declares
int pwmPin = 5; // PWM output pin for motor 1
int dirPin = 8; // direction output pin for motor 1
int sensorPosPin = A2; // input pin for MR sensor
int fsrPin = A3; // input pin for FSR sensor

// Position tracking variables
int updatedPos = 0;     // keeps track of the latest updated value of the MR sensor reading
int rawPos = 0;         // current raw reading from MR sensor
int lastRawPos = 0;     // last raw reading from MR sensor
int lastLastRawPos = 0; // last last raw reading from MR sensor
int flipNumber = 0;     // keeps track of the number of flips over the 180deg mark
int tempOffset = 0;
int rawDiff = 0;
int lastRawDiff = 0;
int rawOffset = 0;
int lastRawOffset = 0;
const int flipThresh = 700;  // threshold to determine whether or not a flip over the 180 degree mark occurred
boolean flipped = false;

// Kinematics variables
double xh = 0;           // position of the handle [m]
double theta_s = 0;      // Angle of the sector pulley in deg
double xh_prev;          // Distance of the handle at previous time step
double xh_prev2;
double dxh;              // Velocity of the handle
double dxh_prev;
double dxh_prev2;
double dxh_filt;         // Filtered velocity of the handle
double dxh_filt_prev;
double dxh_filt_prev2;

// Force output variables
double force = 0;           // force at the handle
double Tp = 0;              // torque of the motor pulley
double duty = 0;            // duty cylce (between 0 and 255)
unsigned int output = 0;    // output command to the motor

//custom params
int dot = 201;
int dash = 601;
int space = 200;
int space_long = 600;
int end_space = 1400;
int SOS[] = {dot, space, dot, space, dot, space_long, dash, space, dash, space, dash, space_long, dot, space, dot, space, dot, end_space};
//int total_seq_time = 0;
//long lastTimer;
//char *SOS[] = {"dot", "space", "dot", "space", "dot", "space_long", "dash", "space", "dash", "space", "dash", "space_long", "dot", "space", "dot", "space", "dot", "end_time"};
int i = 0;
unsigned long previousTime = 0;
unsigned long interval = 1000;
double pulldown_force = 0;
double pulldown_force_stiffness = 100;
double contact_position = 0.02;
double rest_position = -0.01;

// --------------------------------------------------------------
// Setup function -- NO NEED TO EDIT
// --------------------------------------------------------------
void setup() 
{
  // Set up serial communication
  Serial.begin(38400);
  
  // Set PWM frequency 
  setPwmFrequency(pwmPin,1); 
  
  // Input pins
  pinMode(sensorPosPin, INPUT); // set MR sensor pin to be an input
  pinMode(fsrPin, INPUT);       // set FSR sensor pin to be an input

  // Output pins
  pinMode(pwmPin, OUTPUT);  // PWM pin for motor A
  pinMode(dirPin, OUTPUT);  // dir pin for motor A
  
  // Initialize motor 
  analogWrite(pwmPin, 0);     // set to not be spinning (0/255)
  digitalWrite(dirPin, LOW);  // set direction
  
  // Initialize position valiables
  lastLastRawPos = analogRead(sensorPosPin);
  lastRawPos = analogRead(sensorPosPin);

//  for (int i = 0; i < sizeof(SOS)/sizeof(SOS[0]); ++i) {
//    total_seq_time += SOS[i];    
//  }
//  lastTimer = millis();
}


// --------------------------------------------------------------
// Main Loop
// --------------------------------------------------------------
void loop()
{
  
  //*************************************************************
  //*** Section 1. Compute position in counts (do not change) ***  
  //*************************************************************

  // Get voltage output by MR sensor
  rawPos = analogRead(sensorPosPin);  //current raw position from MR sensor

  // Calculate differences between subsequent MR sensor readings
  rawDiff = rawPos - lastRawPos;          //difference btwn current raw position and last raw position
  lastRawDiff = rawPos - lastLastRawPos;  //difference btwn current raw position and last last raw position
  rawOffset = abs(rawDiff);
  lastRawOffset = abs(lastRawDiff);
  
  // Update position record-keeping vairables
  lastLastRawPos = lastRawPos;
  lastRawPos = rawPos;
  
  // Keep track of flips over 180 degrees
  if((lastRawOffset > flipThresh) && (!flipped)) { // enter this anytime the last offset is greater than the flip threshold AND it has not just flipped
    if(lastRawDiff > 0) {        // check to see which direction the drive wheel was turning
      flipNumber--;              // cw rotation 
    } else {                     // if(rawDiff < 0)
      flipNumber++;              // ccw rotation
    }
    if(rawOffset > flipThresh) { // check to see if the data was good and the most current offset is above the threshold
      updatedPos = rawPos + flipNumber*rawOffset; // update the pos value to account for flips over 180deg using the most current offset 
      tempOffset = rawOffset;
    } else {                     // in this case there was a blip in the data and we want to use lastactualOffset instead
      updatedPos = rawPos + flipNumber*lastRawOffset;  // update the pos value to account for any flips over 180deg using the LAST offset
      tempOffset = lastRawOffset;
    }
    flipped = true;            // set boolean so that the next time through the loop won't trigger a flip
  } else {                        // anytime no flip has occurred
    updatedPos = rawPos + flipNumber*tempOffset; // need to update pos based on what most recent offset is 
    flipped = false;
  }
 
  //*************************************************************
  //*** Section 2. Compute position in meters *******************
  //*************************************************************
//SWITCHING CALIBRATION PARAMETERS SIGN
//  double m = -0.01223; // calibration slope
//  double b = 10.44; // calibration offset
  double m = 0.01223; // calibration slope
  double b = -10.44; // calibration offset
  double ts = m*updatedPos + b; // theta in degrees
  double rh = 0.075; //handle radius [m] 
  double xh = rh*(ts/180)*M_PI; // handle x position (rh * ts [in radians])
  // Calculate velocity with loop time estimation
  dxh = (double)(xh - xh_prev) / 0.001;

  // Calculate the filtered velocity of the handle using an infinite impulse response filter
  dxh_filt = .9*dxh + 0.1*dxh_prev; 
    
  // Record the position and velocity
  xh_prev2 = xh_prev;
  xh_prev = xh;
  
  dxh_prev2 = dxh_prev;
  dxh_prev = dxh;
  
  dxh_filt_prev2 = dxh_filt_prev;
  dxh_filt_prev = dxh_filt;
  
  //*************************************************************
  //*** Section 3. Assign a motor output force in Newtons *******  
  //*************************************************************
  //*************************************************************
  //******************* Rendering Algorithms ********************
  //*************************************************************
  Serial.println(xh, 5);   
//  unsigned long currentTime = millis();
  
//  char *current_character = SOS[i];
  int current_character = SOS[i];

  if (current_character == dot){
    interval = dot;
    pulldown_force = pulldown_force_stiffness*(xh - contact_position);
  }
  else if (current_character == dash){
    interval = dash;
    pulldown_force = pulldown_force_stiffness*(xh - contact_position);
  }
  else if (current_character == space){
    interval = space;
    pulldown_force = pulldown_force_stiffness*(xh - rest_position);
  }
  else if (current_character == space_long){
    interval = space_long;
    pulldown_force = pulldown_force_stiffness*(xh - rest_position);
  }
  else {
    interval = end_space;
    pulldown_force = pulldown_force_stiffness*(xh - rest_position);
  }
//  if (xh >= contact_position) {
//    double k = 300.0; //wall stiffness if penetrating wall 
//    force = k*(xh-contact_position) + pulldown_force;
//  }
//  else if (xh >= rest_position || xh < contact_position) {
//    double k = 100;
//    force = k*(xh - rest_position) + pulldown_force;
//  }
//  else {
//    force = 0 + pulldown_force; //free space feels free
//  }
  force = 0 + pulldown_force; //free space feels free
  unsigned long interval_l = (unsigned long) interval;
  unsigned long currentTime = millis();
  if (currentTime - previousTime >= 100*interval_l){
    previousTime = currentTime;
    if (current_character == end_space){
      i = 0;
    }
    else {
      i++;
    }
  }

  

  double rp = 0.00953/2; //  pulley diameter [m] divided by two
  double rs = 0.075;//sector radius [m]
  double J = rh*(rp/rs); //Jacobian
  double Tp = J*force; //output torque
  
  //*************************************************************
  //*** Section 4. Force output (do not change) *****************
  //*************************************************************
  
  // Determine correct direction for motor torque
  if(force > 0) { 
    digitalWrite(dirPin, HIGH);
  } else {
    digitalWrite(dirPin, LOW);
  }

  // Compute the duty cycle required to generate Tp (torque at the motor pulley)
  duty = sqrt(abs(Tp)/0.03);

  // Make sure the duty cycle is between 0 and 100%
  if (duty > 1) {            
    duty = 1;
  } else if (duty < 0) { 
    duty = 0;
  }  
  output = (int)(duty* 255);   // convert duty cycle to output signal
  analogWrite(pwmPin,output);  // output the signal
}

// --------------------------------------------------------------
// Function to set PWM Freq -- DO NOT EDIT
// --------------------------------------------------------------
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
