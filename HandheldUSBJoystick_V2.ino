
//=================================================================================

#include "Joystick.h"

//=================================================================================

Joystick_ Joystick;

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
// WORK IN PROGRESS -- THIS IS NOT WORKING PROPERLY WITH THE XBOX ADAPTIVE CONTROLLER
// THOUGH IT DOES WORK FINE WHEN PLUGGED INTO THE COMPUTER AS A NORMAL JOYSTICK
#define XBOX_AC_STICK_BUTTON   10 

//=================================================================================


void setup() {

  //Startup the joystick object. 
  Joystick.begin(false); // false indicates that sendState method call required (so we do all changes at once).

  //Set HORZ and VERT pins (from joystick) to input
  pinMode(VERT_PIN, INPUT);
  pinMode(HORZ_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

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
  
  Joystick.setXAxis(analogRead(HORZ_PIN));
  Joystick.setYAxis(analogRead(VERT_PIN));

  // ----------------------------------------------------

  // Send updated joystick state to HID upstream
  Joystick.sendState();

}//loop

//=================================================================================

