import processing.serial.*;
import controlP5.*;

Serial myPort;                     // Create object from Serial class
ControlP5 controlP5;               // Create object from menu class
DropdownList p1;                   // Create object from menu class

PImage logo;
PFont myFont;
PFont xyinteractionFont;

boolean TEXTILE_KEYBOARD = true;    // scanne textile keboard
boolean TWITTER = false;            // send Twitter mesage
boolean LOGO = true;                // disply the GE logo

boolean displayKeyboard = false;    // display keboard 16 x 16 matrixe
boolean doDraw = false;             // start by scanning serial port then do drow

boolean DEBUG_state = false;
boolean DEBUG_bytePos = false;
boolean DEBUG_serial = false;
boolean DEBUG_screenPos = false;
boolean DEBUG_letterPos = false;
boolean DEBUG_bitFrame = false;
boolean DEBUG_twitter_id = false;

/////////////////// Setup
void setup() {
  size(1024, 500);
  background(255);
  frameRate(10);
  
  myFont = loadFont("GEInspira-Bold-25.vlw");
  xyinteractionFont = loadFont("Courier-14.vlw"); 

  textFont(myFont);
  controlP5 = new ControlP5(this);
  p1 = controlP5.addDropdownList("myList-p1", width - menuXsize, 20, menuXsize, 500);
  customize(p1);
  if ( TEXTILE_KEYBOARD ) clearKeyboardBuffer();
  if ( LOGO ) logo = loadImage("logo.png");
  if ( TWITTER ) codeSetup();
  if ( TWITTER ) connectTwitter();
  GEframe();
  XYinteraction();
}

void draw() {
  if ( displayKeyboard ) displayKeyboard();
  if ( LOGO ) image(logo, 40, 25);
  refreshMenu();
}
