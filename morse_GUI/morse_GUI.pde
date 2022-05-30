// Morse Code GUI
// ME327 Final Project
// 5/23/22
// Andrew Sack

import processing.serial.*;

// Constants
final int WINDOW_WIDTH = 1280;
final int WINDOW_HEIGHT = 720;
final int CHAR_SIZE = 100;

final int INPUT_HEIGHT = 330; // vertical position of user input line

final int LIGHT_GRAY = 0xd8dce3;
final int DARK_RED = 0xFFad2d2d;
final int BLACK = 0xFF000000;
final int GREEN = 0xFF11d41b;
final int YELLOW = 0xFFfcf514;
final int RED = 0xFFfc0f0f;

final int lf = 10;    // Linefeed in ASCII

// Enum
enum State {
  IDLE,
  WORD_SELECT,
  ROUND1,
  ROUND2,
  ROUND3,
  END
}


// Module Variables
Serial myPort;        // The serial port
StringDict morseDict;
boolean showActualMorse = true;
String currentWord;
int currentLetterIdx;
int currentLetterPosn;
int currentElemIdx;

int highlightIdx = -1;

State currentState;
String serialStr = "";

void settings() {
  // set the window size:
  size(WINDOW_WIDTH, WINDOW_HEIGHT);
  //fullScreen();
}

void setup() { 
  
  // Serial
  // List all the available serial ports
  println(Serial.list());
  // Check the listed serial ports in your machine
  // and use the correct index number in Serial.list()[].
  //note you may need to change port number
  myPort = new Serial(this, Serial.list()[1], 115200);  //port 0 (COM 3) on jack's computer // also make sure baud rate matches Arduino
  
  // A serialEvent() is generated when a newline character is received :
  myPort.bufferUntil('\n');
  
  // Morse Dictionary setup
  initMorseDict();
  
  //currentState  = State.IDLE; //change to word select?
  currentState  = State.WORD_SELECT; //change to word select?
  
  // Slow down draw
  //frameRate(0.5); 
  
  
  drawStaticScreen();
  drawInstructions();
  // Don't constantly run draw
  noLoop();
  
  //String s = "SOSABCD1234";
  //beginUserMorse(s);
  
}

// state machine
void draw() {
  switch(currentState) { //<>//
    case IDLE: {
      // Go to initial state
      if (serialStr.equals("reset")){
        currentState = State.WORD_SELECT;
        drawStaticScreen();
        drawInstructions();
      }
      
    } break;
    
    case WORD_SELECT: {
      showActualMorse = true;
      // ignore reset in this state
      if (serialStr.equals("reset")){
        // do nothing
      }
      // if all upper case, it is an input word
      else if (serialStr.toUpperCase().equals(serialStr)) {
        drawStaticScreen();
        drawInstructions();
        currentWord = serialStr;
        drawWord(serialStr, -1); // no highlighting
      // Start round 1
      } else if (serialStr.equals("start")){
        currentState = State.ROUND1;
        beginUserMorse(currentWord, 1); // begin round 1
      }
      // ignore reset in this state      
      
    } break;
    
    case ROUND1: {
      
      if (serialStr.equals("reset")){
        currentState = State.WORD_SELECT;
        drawStaticScreen();
        drawInstructions();
      
      } else if (serialStr.equals("dot")){
        inputDot();
      } else if (serialStr.equals("dash")){
        inputDash();
      } else if (serialStr.equals("eoc")){
        inputEOC();
      } else if (serialStr.equals("eow")){
        currentState = State.ROUND2;
        beginUserMorse(currentWord, 2); // begin round 2
      } 
      
    } break;
    
    case ROUND2: {
      
      if (serialStr.equals("reset")){
        currentState = State.WORD_SELECT;
        drawStaticScreen();
        drawInstructions();
      } else if (serialStr.equals("dot")){
        inputDot();
      } else if (serialStr.equals("dash")){
        inputDash();
      } else if (serialStr.equals("eoc")){
        inputEOC();
      } else if (serialStr.equals("eow")){
        currentState = State.ROUND3;
        beginUserMorse(currentWord, 3); // begin round 3
      } 
      
    } break;
    
    case ROUND3: {
      
      if (serialStr.equals("reset")){
        currentState = State.WORD_SELECT;
        drawStaticScreen();
        drawInstructions();
      } else if (serialStr.equals("dot")){
        inputDot();
      } else if (serialStr.equals("dash")){
        inputDash();
      } else if (serialStr.equals("eoc")){
        inputEOC();
      } else if (serialStr.equals("eow")){
        currentState = State.END;
        // Go to end state
      } 
      
    } break;
    
    case END: {
      
      if (serialStr.equals("reset")){
        currentState = State.WORD_SELECT;
        drawStaticScreen();
        drawInstructions();
      }
      
    } break;
    
    default:
      println("Unknown State");
  }
  
  // clear string
  serialStr = "";
}


// Draw the static elements of the screen
void drawStaticScreen() {
  background(LIGHT_GRAY); 
  
  // Title
  String title = "TAPKIT: Morse Code Trainer";
  textSize(50);
  textAlign(CENTER, CENTER);
  float tw = textWidth(title);
  fill(DARK_RED);
  text(title, WINDOW_WIDTH/2 , 20);
  
  // Underline
  rectMode(CENTER);
  noStroke();
  rect(WINDOW_WIDTH/2, 50, tw, 5);
  
  ////redraw();
}

// Draw Instructions
void drawInstructions() {
  textSize(20);
  textAlign(LEFT);
  fill(BLACK);
  text("Instructions:", 20, 400);
  text("Word Select:", 30, 425);
  text("Quick-press handle to change selected word", 40, 450);
  text("Hold handle down to start exercise. After 2 sec, return to the neutral position and relax your finger", 40, 475);
  text("During Training:", 30, 500);
  text("There will be 3 rounds, 2 training rounds and 1 test round", 40, 525);
  text("During the training rounds, the TAPKIT will provide force feedback. Let the TAPKIT pull your finger", 40, 550);
  text("During the test round, there will be no force or visual feedback. Move based on what you have learned in the previous rounds", 40, 575);
  
  ////redraw();
}

// Draw specified word
// Highlight character at specified index or none if value is -1
void drawWord(String word, int highlightedChar){
  
  // Draw the word letter by letter
  int x = (width - ((word.length()-1)*CHAR_SIZE))/2;
  
  // Print header for line
  if (showActualMorse){
    fill(BLACK);
    textSize(20);
    textAlign(RIGHT, CENTER);
    text("Actual:",x-75, 300);
    }
  
  for (int i = 0; i < word.length(); i++) {
    if (i == highlightedChar) {
      fill(YELLOW);
    } else {
      fill(BLACK);
    }
    
    // Each character is displayed one at a time with the charAt() function.
    noStroke();
    textSize(CHAR_SIZE);
    textAlign(CENTER, CENTER);
    text(word.charAt(i),x, 200);
    
    // Draw morse for each letter
    if (showActualMorse){
      // Get morse translation of char
      String morse = morseDict.get(String.valueOf(word.charAt(i)));
      for (int j = 0; j < morse.length(); j++) {
        // Dot
        if (morse.charAt(j) == '.') {
          drawDot(x - CHAR_SIZE/2 + 18*(j+1), 300, BLACK);
        } else {
          drawDash(x - CHAR_SIZE/2 + 18*(j+1), 300, BLACK);
        }
      }

    }
    
    // All characters are spaced 10 pixels apart.
    x += CHAR_SIZE;
    
    
  }
  
  ////redraw();
}

// Begin the user morse code input by drawing background and initializing vars
void beginUserMorse(String word, int round)
{
  drawStaticScreen();
  drawInstructions();
  
  // Hide morse truth during last round
  if (round == 3)
  {
    showActualMorse = false;
  } else {
   showActualMorse = true; 
  }
  
  currentWord = word;
  currentElemIdx = 0; // start at 1st morse elem
  currentLetterIdx = 0; // start at 1st letter
  currentLetterPosn = (width - ((currentWord.length()-1)*CHAR_SIZE))/2;
  
  int currentElemPosn = currentLetterPosn - CHAR_SIZE/2 + 18* (currentElemIdx+1);
  
  // draw header
  textSize(20);
  textAlign(RIGHT, CENTER);
  fill(BLACK);
  text("You:",currentLetterPosn-75, INPUT_HEIGHT);
  
  // Draw round number
  String roundStr = "Round: " +  str(round);
  textSize(40);
  textAlign(CENTER, CENTER);
  fill(DARK_RED);
  text(roundStr, WINDOW_WIDTH/2 , 70);
  
  drawWord(currentWord, currentLetterIdx);
  
  ////redraw();
}

// Handles user input of dot
void inputDot() {
  int currentElemPosn = currentLetterPosn - CHAR_SIZE/2 + 18* (currentElemIdx+1);
  int c;
  
  
  // Evaluate input correctness
  String morse = morseDict.get(String.valueOf(currentWord.charAt(currentLetterIdx)));
  if (currentElemIdx < morse.length()){ // compare to corresponding dot/dash
    // set color based on correctness of elem
    if (morse.charAt(currentElemIdx) == '.') {
          c = GREEN;
        } else {
          c = RED;
        }
  } else if (currentElemIdx < 5) { // fits on screen, but always wrong
    c = RED;
  } else { // clipped draw X
    currentElemPosn = currentLetterPosn - CHAR_SIZE/2 + 18* (5+1);
    c = RED;
    drawX(currentElemPosn, INPUT_HEIGHT, c);
    return;
  }
  
  drawDot(currentElemPosn, INPUT_HEIGHT, c);
  currentElemIdx ++; 
}

// Handles user input of dash
void inputDash() {
  int currentElemPosn = currentLetterPosn - CHAR_SIZE/2 + 18* (currentElemIdx+1);
  int c;
  
    // Evaluate input correctness
  String morse = morseDict.get(String.valueOf(currentWord.charAt(currentLetterIdx)));
  if (currentElemIdx < morse.length()){ // compare to corresponding dot/dash
    // set color based on correctness of elem
    if (morse.charAt(currentElemIdx) == '-') {
          c = GREEN;
        } else {
          c = RED;
        }
  } else if (currentElemIdx < 5) { // fits on screen, but always wrong
    c = RED;
  } else { // clipped draw X
    currentElemPosn = currentLetterPosn - CHAR_SIZE/2 + 18* (5+1);
    c = RED;
    drawX(currentElemPosn, INPUT_HEIGHT, c);
    return;
  }
  
  drawDash(currentElemPosn, INPUT_HEIGHT, c);
  currentElemIdx++; 
}

// Handles user input of End of Character
void inputEOC() {
  currentLetterIdx++;
  currentLetterPosn = (width - ((currentWord.length()-1)*CHAR_SIZE))/2 + (currentLetterIdx * CHAR_SIZE);
  currentElemIdx = 0;
  
  if (currentLetterIdx >= currentWord.length())
  {
    println("Finished Word");
    currentLetterIdx = 0;
  } else {
   drawWord(currentWord, currentLetterIdx); 
  }
}

// Handles user input of End of Word
void inputEOW() {
  println("Morse input round ended");
  //exit();
}

// Draw morse dot centered at x,y with color c
void drawDot(int x, int y, int c){
  noStroke();
  ellipseMode(CENTER);
  fill(c);
  ellipse(x, y, 12, 12);
  
  ////redraw();
}

// Draw morse dash centered at x,y with color c
void drawDash(int x, int y, int c){
  noStroke();
  rectMode(CENTER);
  fill(c);
  rect(x, y, 12, 5);
  
  ////redraw();
}

// Draw morse X centered at x,y with color c
void drawX(int x, int y, int c){
  noStroke();
  textAlign(RIGHT, CENTER);
  fill(c);
  textSize(12);
  text("X", x-2, y-2);
  //redraw();
}


// init morse dict and insert all keys
void initMorseDict(){
  morseDict = new StringDict();
  
  morseDict.set("A", ".-");
  morseDict.set("B", "-...");
  morseDict.set("C", "-.-.");
  morseDict.set("D", "-..");
  morseDict.set("E", ".");
  
  morseDict.set("F", "..-.");
  morseDict.set("G", "--.");
  morseDict.set("H", "....");
  morseDict.set("I", "..");
  morseDict.set("J", ".---");
  
  morseDict.set("K", "-.-");
  morseDict.set("L", ".-..");
  morseDict.set("M", "--");
  morseDict.set("N", "-.");
  morseDict.set("O", "---");
  
  morseDict.set("P", ".--.");
  morseDict.set("Q", "--.-");
  morseDict.set("R", ".-.");
  morseDict.set("S", "...");
  morseDict.set("T", "-");
  
  morseDict.set("U", "..-");
  morseDict.set("V", "...-");
  morseDict.set("W", ".--");
  morseDict.set("X", "-..-");
  morseDict.set("Y", "-.--");
  morseDict.set("Z", "--..");
  
  morseDict.set("1", ".----");
  morseDict.set("2", "..---");
  morseDict.set("3", "...--");
  morseDict.set("4", "....-");
  morseDict.set("5", ".....");
  
  morseDict.set("6", "-....");
  morseDict.set("7", "--...");
  morseDict.set("8", "---..");
  morseDict.set("9", "----.");
  morseDict.set("0", "-----");

}

// Simulate serial with keyboard presses
void keyPressed() {
  if (key == 'a') { // dot
    inputDot();
    //serialStr = "dot";
  } else if (key == 's'){ // dash
    inputDash();
    //serialStr = "dash";
  } else if (key == 'd'){ // eoc
    inputEOC();
    //serialStr = "eoc";
  } else if (key == 'f'){ // eow
    inputEOW();
    //serialStr = "eow";
  } else {
   return; // return before redraw 
  }
  redraw();
}

void serialEvent (Serial myPort) {
  // Read in String from serial
  String tempSerialStr = myPort.readStringUntil('\n'); 
  // return if Null
  if (serialStr == null){
    return;
  }
  // trim whitespace off end
  tempSerialStr = trim(tempSerialStr);
  // make sure it's not empty string
  if (tempSerialStr.length() >=1)
  {
    serialStr = tempSerialStr;
    redraw();
  }
  
}
