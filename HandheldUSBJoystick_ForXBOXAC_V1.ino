
//=================================================================================

#include "Joystick.h"

//=================================================================================

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_JOYSTICK, 1, 0,
  true, true, false, false, false, false,
  false, false, false, false, false);

//=================================================================================

const boolean debug = true;

//=================================================================================

//Joystick variables - Button (with debounce)
int SEL, prevSEL = HIGH;
long lastDebounceTime = 0;
long debounceDelay = 50;  //millis

//=================================================================================

//Joystick pins
#define VERT_PIN    A9 // AKA the "Y" -- dependent on how thumbstick is oriented
#define HORZ_PIN    A8 // AKA the "X" -- dependent on how thumbstick is oriented
#define BUTTON_PIN  7  // Pushbutton of the thumbstick

//Mapping to XBOX Adaptive Controller
//X and Y will auto map to Axis 0/1 or 2/3 depending on which side you plug
//the joystick into (left=0/1 or right=2/3). The pushbutton on the stick needs
//to be set & programmed depending on placement. TODO: Some sort of config on stick?
//10=Left Stick Press, 11=Right Stick Press
#define XBOX_AC_STICK_BUTTON   10 

//=================================================================================

int rawHorz, rawVert;
int mapHorz, mapVert;

//invert if needed
bool invHorz = false;
bool invVert = true;

//=================================================================================


void setup() {

  //Startup the joystick object. 
  Joystick.begin(false); // false indicates that sendState method call required (so we do all changes at once).

  //I'm using Map below, but setting range Just In Case.
  //Also setting Axis to zero at startup.
  Joystick.setXAxisRange(-127, 127); Joystick.setXAxis(0);
  Joystick.setYAxisRange(-127, 127); Joystick.setYAxis(0);
  Joystick.sendState();


  //Set HORZ and VERT pins (from joystick) to input
  pinMode(VERT_PIN, INPUT);
  pinMode(HORZ_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (debug) { 
    Serial.begin(9600);
    Serial.println("HandheldUSBJoystick_ForXBOXAC_V1");
  }

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

  //Read analog values from input pins.
  rawHorz = analogRead(HORZ_PIN);
  rawVert = analogRead(VERT_PIN);

  //Map values to a range the XAC likes
  mapHorz = map(rawHorz, 0, 1023, -127, 127);
  mapVert = map(rawVert, 0, 1023, -127, 127);

  if (debug) {
    Serial.print("RAW H/V: ");
    Serial.print(rawHorz);
    Serial.print(" ");
    Serial.print(rawVert);
    Serial.print("     ");
    Serial.print("MAP H/V: ");
    Serial.print(mapHorz);
    Serial.print(" ");
    Serial.print(mapVert);
    Serial.println();
  }

  if (invHorz) {mapHorz = -mapHorz;}
  if (invVert) {mapVert = -mapVert;}
  
  Joystick.setXAxis(mapHorz);
  Joystick.setYAxis(mapVert);

  // ----------------------------------------------------

  // Send updated joystick state to HID upstream
  Joystick.sendState();

}//loop

//=================================================================================
