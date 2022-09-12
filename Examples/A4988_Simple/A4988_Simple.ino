#include "A4988_Stepper.h"

#define PIN_STEP 6
#define PIN_DIR  2
#define PIN_ENABLE 8
#define PIN_FEEDBACK 7
#define PIN_MS1 3
#define PIN_MS2 4
#define PIN_MS3 5

int resolution = SR16;

bool dir = CCW;
bool lock = false;

long SpeedVal = 2400;
long TurnsVal = 41;
long AccelVal = 5;


A4988_Stepper Nema17(PIN_STEP, PIN_DIR, PIN_ENABLE, PIN_FEEDBACK, PIN_MS1, PIN_MS2, PIN_MS3);

// no need for speed control or enable?
//A4988_Stepper Nema17(PIN_STEP, PIN_DIR, PIN_FEEDBACK);

elapsedMillis tt;

void setup() {

  Serial.begin(115200);

  Nema17.begin(400, 4);

  // if you are hardwiring your stepper for other that full (1/2, 1/4 etc.)
  // you will need to call this to tell the library what the resolution is
  //Nema17.setStepResolution(SR01);

  // if you using a stepper that is not 200 steps per rev
  // you will need to call this to tell the library what the steps are
  //Nema17.setStepsPerRev(200); // 200 for standard NEMA17, 20 for DVD motor


  // make your forst move
  Nema17.moveRevolutions(20); 		// should rotate 20 turns

  // non blocking so you will need to add your own wait/reporting system
  tt = 0; // reset the elapsed timer
  while (Nema17.IsRunning) {
    if (tt > 500) { // update every 50 ms
      tt = 0;
      Serial.print("Motor RPM "); Serial.println(Nema17.getRPM());
      Serial.print("Revolutions "); Serial.println(Nema17.getRevolutions());
      Serial.print("Steps "); Serial.println(Nema17.getAbsoluteSteps());
    }
  }

  // now reverse direction back 400 steps (2 revolutions)
  Nema17.moveSteps(-400);			// should rotate back 429 steps
  // non blocking so you will need to add your own wait/reporting system
  tt = 0; // reset the elapsed timer
  while (Nema17.IsRunning) {
    if (tt > 10) { // update every 10 ms
      tt = 0;
      Serial.print("Motor RPM "); Serial.println(Nema17.getRPM());
      Serial.print("Revolutions "); Serial.println(Nema17.getRevolutions());
      Serial.print("Steps "); Serial.println(Nema17.getAbsoluteSteps());
    }
  }
}

void loop() {

}