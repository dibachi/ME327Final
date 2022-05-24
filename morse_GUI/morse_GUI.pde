// Morse Code GUI
// ME327 Final Project
// 5/23/22
// Andrew Sack

import processing.serial.*;

// Constants
final int WINDOW_WIDTH = 1280;
final int WINDOW_HEIGHT = 720;
final int CHAR_SIZE = 100;

final int LIGHT_GRAY = 0xd8dce3;
final int RED = 0xFFad2d2d;
final int BLACK = 0xFF000000;
final int GREEN = 0xFF11d41b;

// Module Variables
Serial myPort;        // The serial port
StringDict morseDict;
boolean showActualMorse = true;

int highlightIdx = -1;

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
  //myPort = new Serial(this, Serial.list()[1], 38400);  // also make sure baud rate matches Arduino
  // A serialEvent() is generated when a newline character is received :
  //myPort.bufferUntil('\n');
  
  // Morse Dictionary setup
  initMorseDict();
  
  
  
  // Slow down draw
  //frameRate(0.5); 
  
  drawStaticScreen();
  
  
}

void draw() {
  drawStaticScreen();
  drawInstructions();
  
  String s = "SOSABCD1234";
  if(highlightIdx >= s.length()) {
    highlightIdx = -1;
  }
  
  drawWord(s, highlightIdx);
  delay(1000);
  highlightIdx++;
  
  
  
}

// Draw the static elements of the screen
void drawStaticScreen() {
  background(LIGHT_GRAY); 
  
  // Title
  String title = "Morse Code Trainer";
  textSize(50);
  textAlign(CENTER, CENTER);
  float tw = textWidth(title);
  fill(RED);
  text(title, WINDOW_WIDTH/2 , 20);
  
  // Underline
  rectMode(CENTER);
  noStroke();
  rect(WINDOW_WIDTH/2, 50, tw, 5);
}

// Draw Instructions
void drawInstructions() {
  textSize(20);
  textAlign(LEFT);
  fill(BLACK);
  text("Instructions:", 20, 400);
  text("Quick-press handle to change selected word", 20, 425);
  text("Hold handle down to start exercise", 20, 450);
}

// Draw specified word
// Highlight character at specified index or none if value is -1
void drawWord(String word, int highlightedChar){
  
  
  // Draw the word letter by letter
  int x = (width - ((word.length()-1)*CHAR_SIZE))/2;
  
  // Print header for line
  if (showActualMorse){
    textSize(20);
    textAlign(RIGHT, CENTER);
    text("Actual:",x-75, 300);
    }
  
  for (int i = 0; i < word.length(); i++) {
    if (i == highlightedChar) {
      fill(GREEN);
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
}

// Draw morse dot centered at x,y with color c
void drawDot(int x, int y, int c){
  noStroke();
  ellipseMode(CENTER);
  fill(c);
  ellipse(x, y, 12, 12);
  
}

// Draw morse dash centered at x,y with color c
void drawDash(int x, int y, int c){
  noStroke();
  rectMode(CENTER);
  fill(c);
  rect(x, y, 12, 5);
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
