//--------------------------------------------------------------------------
// Morse Code
// Get it? Morse code? Because it's code for doing Morse code!
// with Andrew's edits
//--------------------------------------------------------------------------

/*PSEUDOCODE
at all times, have restoring force to rest and virtual wall force
if tapper position is in contact with the virtual wall,
  buzzer and led are on
  register time since contact began
else
  buzzer and led are off
  register time when contact ended

word selection
  if the contact time was brief,
    increment list once and send selected word
  if the contact time was lengthy,
    send 'start'
    leave word selection and go to stage 1

loop between stages
  if stage1, 
    pulldown force = highest
  if stage2,
    pulldown force = medium
  if stage3,
    pulldown force = lowest
  if contact time was brief, 
    send 'dot'
  if contact time was lengthy,
    send 'dash'
  if off-time is lengthy,
    send 'eoc'
  if stage is done (end of morse sequence),
    send 'eow'
    if remaining stages,
      move to next stage
    else,
      go to end state
      leave loop

if at end state,
  hang out here for some amount of time
  if amount of time has elapsed,
    leave end state and go back to word selection
    send 'reset'
  


*/




// Includes
#include <math.h>

// Pin declares
int pwmPin = 5; // PWM output pin for motor 1
int dirPin = 8; // direction output pin for motor 1
int sensorPosPin = A2; // input pin for MR sensor
int fsrPin = A3; // input pin for FSR sensor
int buzzerPin = 9; //?
int ledPin = 10; //?

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
int unit_time = 150;
int dot = unit_time + 1;
int dash = 3*unit_time + 1;
int space = unit_time;
int space_long = 3*unit_time;
int end_space = 7*unit_time;
int morse_sequence[100] = {};
// int morse_sequence;
int SOS[] = {dot, space, dot, space, dot, space_long, dash, space, dash, space, dash, space_long, dot, space, dot, space, dot, end_space};
int HAPTICS[] = {dot, space, dot, space, dot, space, dot, space_long, dot, space, dash, space_long, dot, space, dash, space, dash, space, dot, space_long, dash, space_long, dot, space, dot, space_long, dash, space, dot, space, dash, space, dot, space_long, dot, space, dot, space, dot, end_space};
int STANFORD[] = {dot, space, dot, space, dot, space_long, dash, space_long, dot, space, dash, space_long, dash, space, dot, space_long, dot, space, dot, space, dash, space, dot, space_long, dash, space, dash, space, dash, space_long, dot, space, dash, space, dot, space_long, dot, space, dash, space, dot, space_long, dash, space, dot, space, dot, end_space};
int word_selection = 0;
int prev_word_selection = 2;
//int total_seq_time = 0;
//long lastTimer;
//char *morse_sequence[] = {"dot", "space", "dot", "space", "dot", "space_long", "dash", "space", "dash", "space", "dash", "space_long", "dot", "space", "dot", "space", "dot", "end_time"};
int i = 0;
unsigned long currentTimeContact = 0;
unsigned long currentTimeRelease = 0;
unsigned long previousTimePull = 0;
unsigned long previousTimeContact = 0;
unsigned long previousTimeRelease = 0;
unsigned long intervalPull = 1000;
unsigned long intervalContact = 0;
unsigned long intervalRelease = 0;
unsigned long intervalContact_l = (unsigned long) 100*intervalContact;
unsigned long intervalRelease_l = (unsigned long) 100*intervalRelease;
double pulldown_force = 0;
double pulldown_force_stiffness = 200;
double contact_position = 0.0;
double rest_position = -0.01; //-0.01
double contact_force = 0;
double contact_stiffness = 1000;
double restoring_force = 0;
double restoring_stiffness = 100;
unsigned long timing_tolerance = 100;

boolean already_printed_start = false;
boolean already_printed_dot = false;
boolean already_printed_dash = false;
boolean already_printed_eoc = false;
boolean already_printed_eow = false;

boolean contact_state = false;
boolean prev_contact_state = false;
unsigned long currentTime = 0;
unsigned long prevTime = 0;
unsigned long timeInterval = 0;

// enum machine_state { waiting, stage1, stage2, stage3, freewheel};
// machine_state state;
// machine_state prev_state;
int state = 0; //word selection
int prev_state = 0; //word selection
// --------------------------------------------------------------
// Setup function -- NO NEED TO EDIT
// --------------------------------------------------------------
void setup() 
{
  // Set up serial communication
  Serial.begin(115200); //JUST CHANGED
  
  // Set PWM frequency 
  setPwmFrequency(pwmPin,1); 
  
  // Input pins
  pinMode(sensorPosPin, INPUT); // set MR sensor pin to be an input
  pinMode(fsrPin, INPUT);       // set FSR sensor pin to be an input

  // Output pins
  pinMode(pwmPin, OUTPUT);  // PWM pin for motor A
  pinMode(dirPin, OUTPUT);  // dir pin for motor A

  //peripherals
  pinMode(buzzerPin, OUTPUT); 
  pinMode(ledPin, OUTPUT);
  analogWrite(buzzerPin, 0);
  analogWrite(ledPin, 0);
  
  // Initialize motor 
  analogWrite(pwmPin, 0);     // set to not be spinning (0/255)
  digitalWrite(dirPin, LOW);  // set direction
  
  // Initialize position valiables
  lastLastRawPos = analogRead(sensorPosPin);
  lastRawPos = analogRead(sensorPosPin);

  state = 0;
  prev_state = 0;
  intervalContact = 0;
  intervalRelease = 0;
  intervalContact_l = (unsigned long) 100*intervalContact;
  intervalRelease_l = (unsigned long) 100*intervalRelease;
  //Serial.println("setup");

  // state = waiting;
  // prev_state = waiting;
//  for (int i = 0; i < sizeof(morse_sequence)/sizeof(morse_sequence[0]); ++i) {
//    total_seq_time += morse_sequence[i];    
//  }
//  lastTimer = millis();
}

// void change(int **array, int length)
// {
//     free(*array);
//     *array = malloc(length * sizeof(int));
//     if (*array == NULL)
//         return;
//     for (int i = 0 ; i < length ; i++)
//         (*array)[i] = 1;
// }
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
  
  //************************ PRINTING ***************************
//  Serial.println(xh, 5);   
//  unsigned long currentTimePull = millis();
  
//  char *current_character = morse_sequence[i];
  //handles pulldown force for training arbitrary message in three stages
  // if (state != waiting or state != freewheel){
  
  //NEW CODE BEGINS (KEEP IT CLEAN DICKHEAD)
  if (xh > contact_position) {
    analogWrite(buzzerPin, 50); //buzzer
    analogWrite(ledPin, 255); //led
    contact_force = contact_stiffness*(xh - contact_position);
    restoring_force = 0;

    contact_state = true;
    
  } else {
    analogWrite(buzzerPin, 0); //buzzer
    analogWrite(ledPin, 0); //led
    contact_force = 0; //virtual wall off
    restoring_force = restoring_stiffness*(xh - rest_position);
    
    contact_state = false;
  }

  if (contact_state != prev_contact_state) {
    // A change has occurred
    currentTime = millis();
    timeInterval = currentTime - prevTime;
    prevTime = currentTime;

    if (contact_state == false) {
      intervalContact_l = timeInterval;
      intervalRelease_l = 0;
    }
    if (contact_state == true) {
      intervalRelease_l = timeInterval;
    }
    
    prev_contact_state = contact_state;
  }

  if (state == 0) { //if in word selection phase
//    Serial.println(word_selection);
    if (intervalContact_l < (unsigned long) 100*(dot + timing_tolerance) && intervalContact_l != 0) {
      if (word_selection >= 2) {
        word_selection = 0;
      } else{
        word_selection++;
      } //end word selection increment
    } //end brief tap in word selection
    if (word_selection == 1) {
//      Serial.println("reset");
//      Serial.println("HAPTICS");
//      delay(250);
      if (prev_word_selection != word_selection) {
        Serial.println("reset");
        delay(50);
        Serial.println("HAPTICS");
        for (int j = 0; j<sizeof(HAPTICS)/sizeof(int); j++) {
          morse_sequence[j] = HAPTICS[j];
        }
        prev_word_selection = word_selection;
      }
      // morse_sequence = HAPTICS;
    } else if (word_selection == 2) {
//      Serial.println("reset");
//      Serial.println("STANFORD");
//      delay(250);
      if (prev_word_selection != word_selection) {
        Serial.println("reset");
        delay(50);
        Serial.println("STANFORD");
        for (int j = 0; j<sizeof(STANFORD)/sizeof(int); j++) {
          morse_sequence[j] = STANFORD[j];
        }
        prev_word_selection = word_selection;
      }
      // morse_sequence = STANFORD;
    } else {
//      Serial.println("reset");
//      Serial.println("SOS");
//      delay(250);
      if (prev_word_selection != word_selection) {
        Serial.println("reset");
        delay(50);
        Serial.println("SOS");
        for (int j = 0; j<sizeof(SOS)/sizeof(int); j++) {
          morse_sequence[j] = SOS[j];
        }
        prev_word_selection = word_selection;
      }
      // morse_sequence = SOS; //need a pointer
    } //end word and sequence selection
    //moved word selection before print
//    if (intervalContact_l < (unsigned long) 100*(dot + timing_tolerance)&& intervalContact_l != 0) {
//      if (word_selection >= 2) {
//        word_selection = 0;
//      } else{
//        word_selection++;
//      } //end word selection increment
//    } //end brief tap in word selection
    if (intervalContact_l > (unsigned long) 100*500) {
      Serial.println("start");
      delay(200);
      state = 1; //begin stage 1
    } //end long word selection
  } //end word selection

  if (state != 0 && state != 4) { //if state is not word selection nor end
    if (state == 1) { //set pulldown stiffness 
      pulldown_force_stiffness = 200;
    }
    if (state == 2) { //set pulldown stiffness
      pulldown_force_stiffness = 150;
    }
    if (state == 3) { //set pulldown stiffness
      pulldown_force_stiffness = 100;
    }

    int current_character = morse_sequence[i]; //current dot/dash/space

    //set interval length and pulldown force given position in morse sequence
    if (current_character == dot){ 
      intervalPull = dot;
      pulldown_force = pulldown_force_stiffness*(xh - contact_position);
    }
    else if (current_character == dash){
      intervalPull = dash;
      pulldown_force = pulldown_force_stiffness*(xh - contact_position);
    }
    else if (current_character == space){
      intervalPull = space;
      pulldown_force = pulldown_force_stiffness*(xh - rest_position);
    }
    else if (current_character == space_long){
      intervalPull = space_long;
      pulldown_force = pulldown_force_stiffness*(xh - rest_position);
    }
    else {
      intervalPull = end_space;
      pulldown_force = pulldown_force_stiffness*(xh - rest_position);
    } //end interval length and pulldown force setting
    
    //dot/dash/eoc send
    if (intervalContact_l < 100*(dot + timing_tolerance) && intervalContact_l > 100*(dot - timing_tolerance) && intervalContact_l != 0) {
      if (!already_printed_dot){
        Serial.println("dot");
        already_printed_dot = true;
//        Serial.println(intervalContact_l);
      }
    } //end dot send
    else if (intervalContact_l > 100*(dash - 2*timing_tolerance) && intervalContact_l < 100*(dash + 2*timing_tolerance)&& intervalContact_l != 0) {
      if (!already_printed_dash) {
        Serial.println("dash");
        already_printed_dash = true;
//        Serial.println(intervalContact_l);
      }
    } //end dash send
    else {
      
    }
//    if (intervalRelease_l < 100*(space_long + 2*timing_tolerance) && intervalRelease_l > 100*(space_long - 2*timing_tolerance) && intervalRelease_l != 0) {
//      if (!already_printed_eoc) {
//        Serial.println("eoc");
//        already_printed_eoc = true;
//      }
//    } //end eoc send
    

    //pull timing
    unsigned long intervalPull_l = (unsigned long) intervalPull;
    unsigned long currentTimePull = millis();
    //if pull/push time elapsed
    if (currentTimePull - previousTimePull >= 100*intervalPull_l){
      previousTimePull = currentTimePull;
      already_printed_dot = false;
      already_printed_dash = false;
//      already_printed_eoc = false;
      // Serial.println(intervalPull_l);
      if (current_character == end_space){ //end of morse sequence
        i = 0; //back to beginning of sequence
        prev_state = state; 
        Serial.println("eow");
        state++; //move on to next stage
      }
      else {
        if (current_character == space_long) {
          Serial.println("eoc");
        }
        i++; //move on to next dot/dash/space
      } //end if current_character == end_space
    } //end if interval elapsed
    //end pull timing 
  } //end if state != 0 and state != 4
  if (state == 4) {
    //hang out here for a bit?
    Serial.println("reset");
    state = 0; //back to waiting state
  }
  intervalContact_l = 0;
  intervalRelease_l = 0;
  force = restoring_force + contact_force + pulldown_force;
  //NEW CODE ENDS

  

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
