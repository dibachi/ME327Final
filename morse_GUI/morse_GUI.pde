// Morse Code GUI
// ME327 Final Project
// 5/23/22
// Andrew Sack

import processing.serial.*;

// Constants
final int WINDOW_WIDTH = 1280;
final int WINDOW_HEIGHT = 720;
final int LIGHT_GRAY = 0xd8dce3;

// Module Variables
Serial myPort;        // The serial port

void settings() {
  // set the window size:
  size(WINDOW_WIDTH, WINDOW_HEIGHT);
  //fullScreen();
}

void setup() { 
  // List all the available serial ports
  println(Serial.list());
  // Check the listed serial ports in your machine
  // and use the correct index number in Serial.list()[].
  //note you may need to change port number
  //myPort = new Serial(this, Serial.list()[1], 38400);  // also make sure baud rate matches Arduino
  

  // A serialEvent() is generated when a newline character is received :
  //myPort.bufferUntil('\n');
  
  drawStaticScreen();
}

void draw() {
  if (mousePressed) {
    fill(0);
  } else {
    fill(255);
  }
  ellipse(mouseX, mouseY, 80, 80);
}

// Draw the static elements of the screen
void drawStaticScreen() {
  background(LIGHT_GRAY); 
}
