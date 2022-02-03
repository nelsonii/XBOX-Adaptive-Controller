//JoyConStick_Converter_ForXBOXAC_v2D

//=================================================================================

#include "Joystick.h"

//=================================================================================

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_JOYSTICK, 1, 0,
  true, true, false, false, false, false,
  false, false, false, false, false);

// constructor: use default id, it's a joystick, there is one button,
//              there are no hat buttons, there is an X and Y analog
//              and no other controls.

//=================================================================================

const boolean debug = false; // if true, output info to serial monitor.
const boolean graph = false; // if true, output data for plotter/graph viewing.

//=================================================================================

// Joystick pins
#define VERT_PIN    A1 // physical pin 19 // AKA the "Y" -- dependent on how thumbstick is oriented
#define HORZ_PIN    A2 // physical pin 20 // AKA the "X" -- dependent on how thumbstick is oriented
#define BUTTON_PIN  A3 // physical pin 21 // Pushbutton of the thumbstick

// Old-school Atari digital up/down/left/right buttons
// Not currently used, but wired in my test setup
const int DIG_UP = 4;
const int DIG_DN = 5;
const int DIG_LT = 6;
const int DIG_RT = 7;  

// Mapping to XBOX Adaptive Controller
// X and Y will auto map to Axis 0/1 or 2/3 depending on which side you plug
// the joystick into (left=0/1 or right=2/3). The pushbutton on the stick needs
// to be set & programmed depending on placement. 
// ********* TODO: Some sort of config on stick?
// 10=Left Stick Press, 11=Right Stick Press
int XBOX_AC_STICK_BUTTON = 10;

// Joystick variables - Button (with debounce)
int SEL, prevSEL = HIGH;
long lastDebounceTime = 0;
const int debounceDelay = 50;  //millis

//=================================================================================

// values from analog stick
long rawHorz, rawVert;
long mapHorz, mapVert;

// invert if needed
const bool invHorz = true;
const bool invVert = true;

//=================================================================================

// The XBOX Adapative Controller (XAC) likes joystick values from -127 to + 127
// adjust these min/max based on your upstream device. Same values for both X and Y.
const int joyMin = -127;
const int joyMax = +127;

//=================================================================================

// set the default min/max values.
// Determined during testing. These are for JoyCon Switch style joysticks.
// Adjust values based on what you actually get from the stick being used.

int minHorz=114;
int centeredHorz=485;
int maxHorz=866;

int minVert=114;
int centeredVert=489;
int maxVert=799;

//hard limits, to catch out-of-range readings
const int limitMinHorz=100;
const int limitMaxHorz=900;
const int limitMinVert=100;
const int limitMaxVert=900;

//=================================================================================


void setup() {

  // Startup the joystick object. 
  Joystick.begin(false); // false indicates that sendState method call required (so we do all changes at once).

  // I'm using Map below, but setting range Just In Case.
  // Also setting Axis to zero at startup.
  Joystick.setXAxisRange(joyMin, joyMax); Joystick.setXAxis(0);
  Joystick.setYAxisRange(joyMin, joyMax); Joystick.setYAxis(0);
  Joystick.sendState();

  // Set HORZ and VERT and BUTT pins (from joystick) to input
  pinMode(VERT_PIN, INPUT);
  pinMode(HORZ_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (debug) { 
    Serial.begin(9600);
    Serial.println("JoyConStick_Converter_ForXBOXAC_v2p1");
  }

  // On startup, take some readings from the (hopefully) 
  // centered joystick to determine actual center for X and Y. 
  int iNumberOfSamples = 25;
  long lSampleSumHorz = 0;
  long lSampleSumVert = 0;
  for (int iSample = 1; iSample<=iNumberOfSamples; iSample++) {
    lSampleSumHorz += analogRead(HORZ_PIN); delay(10);
    lSampleSumVert += analogRead(VERT_PIN); delay(10);
  }
  centeredHorz=int(lSampleSumHorz/iNumberOfSamples);
  centeredVert=int(lSampleSumVert/iNumberOfSamples);

}//setup


//=================================================================================


void loop() {

  // ----------------------------------------------------

  // Check for button push - and debounce
  int reading = digitalRead(BUTTON_PIN);
  if (reading != prevSEL) {
     lastDebounceTime = millis();// reset timer
  }
  if (millis() - lastDebounceTime > debounceDelay) {
    if (reading != SEL) {
      SEL = reading;
      if (SEL == LOW) {Joystick.pressButton(XBOX_AC_STICK_BUTTON);}
      else {Joystick.releaseButton(XBOX_AC_STICK_BUTTON);}
    }
  }
  prevSEL = reading;

  // ----------------------------------------------------

  // Read analog values from input pins.
  rawHorz = analogRead(HORZ_PIN); delay(10);
  rawVert = analogRead(VERT_PIN); delay(10);

  // Toss out invalid ranges (out of limit). If out of range, set to centered value.
  if (rawHorz<limitMinHorz || rawHorz>limitMaxHorz) {rawHorz=centeredHorz;}
  if (rawVert<limitMinVert || rawVert>limitMaxVert) {rawVert=centeredVert;}
  
  // update the min/max during run, in case we see something outside of the normal
  if (rawHorz<minHorz) {minHorz=rawHorz;}
  if (rawHorz>maxHorz) {maxHorz=rawHorz;}
  if (rawVert<minVert) {minVert=rawVert;}
  if (rawVert>maxVert) {maxVert=rawVert;}

  // Map values to a range the upstread device likes
  mapHorz = map(rawHorz, minHorz, maxHorz, joyMin, joyMax);
  mapVert = map(rawVert, minVert, maxVert, joyMin, joyMax);

  // Clean up center position, the 5 and 15 are based on value seen during testing.
  if (abs(mapHorz)>0 && abs(mapHorz)<5) {mapHorz=0;}
  if (abs(mapVert)>0 && abs(mapVert)<10) {mapVert=0;}
  
  // Invert value if requested (if "up" should go "down" or "left" to "right")
  if (invHorz) {mapHorz = -mapHorz;}
  if (invVert) {mapVert = -mapVert;}

  // Send the values to the joystick object
  Joystick.setXAxis(mapHorz);
  Joystick.setYAxis(mapVert);

  // Send updated joystick state to HID upstream
  Joystick.sendState();

  // ----------------------------------------------------

  if (debug) {serialDebug();}

}//loop

//=================================================================================

void serialDebug() {
  
    if (!graph) {
      Serial.print("H: ");
      Serial.print(minHorz);
      Serial.print(" ");
      Serial.print(centeredHorz);
      Serial.print(" ");
      Serial.print(maxHorz);
      Serial.print(" ");
      Serial.print("V: ");
      Serial.print(minVert);
      Serial.print(" ");
      Serial.print(centeredVert);
      Serial.print(" ");
      Serial.print(maxVert);
      Serial.print(" ");
      Serial.print("Raw H/V: ");
      Serial.print(rawHorz);
      Serial.print(" ");
      Serial.print(rawVert);
      Serial.print(" ");
    }
    if (!graph) {Serial.print("Button: ");}
    //always graph/print button
    Serial.print(digitalRead(BUTTON_PIN));
    Serial.print(" ");
    if (!graph) {Serial.print("Map H/V: ");}
    //always graph/print mapped horz/vert
    Serial.print(mapHorz);
    if (invHorz) { Serial.print("i"); } // i indicates value was inverted 
    Serial.print(" ");
    Serial.print(mapVert);
    if (invVert) { Serial.print("i"); } // i indicates value was inverted
    Serial.print(" ");
    Serial.println();

}//serialDebug
