#include "A4988_Stepper.h"
#include <ILI9341_t3.h>           // fast display driver lib
#include <ILI9341_t3_Controls.h>  // custom control define file
#include <font_Arial.h>
#include <Colors.h>
#include <XPT2046_Touchscreen.h>
#include <FlickerFreePrint.h>     // library to draw w/o flicker

#define FONT Arial_16
#define TFT_DC 9                  // DC pin on LCD
#define TFT_CS 10                 // chip select pin on LCD
#define LCD_PIN A9                // lcd pin to control brightness
#define TFT_RST 1

int BtnX, BtnY;
int OptionA0ID, OptionA1ID;
int OptionR0ID, OptionR1ID, OptionR2ID, OptionR3ID, OptionR4ID;
boolean istouched;
int resolution = SR16;
bool DrawBTN = false;
bool dir = CCW;
bool lock = false;
char buf[32];
long SpeedVal = 2400;
long TurnsVal = 41;
long AccelVal = 5;

ILI9341_t3 Display = ILI9341_t3(TFT_CS, TFT_DC);

XPT2046_Touchscreen Touch(A0, A1);
TS_Point p;
A4988_Stepper Nema17(6, 2, 8, 7, 3, 4, 5);
SliderH Speed(&Display);
SliderH Turns(&Display);
SliderH Accel(&Display);
OptionButton Direction(&Display);
OptionButton Resolution(&Display);
Button Button1(&Display);
Button Button2(&Display);
CheckBox Lock(&Display);

FlickerFreePrint<ILI9341_t3> SpeedText(&Display, C_WHITE, C_BLACK);
FlickerFreePrint<ILI9341_t3> TurnText(&Display, C_WHITE, C_BLACK);
FlickerFreePrint<ILI9341_t3> AccelText(&Display, C_WHITE, C_BLACK);
elapsedMillis tt;

void setup() {

  Serial.begin(115200);
  while(!Serial){}

  pinMode(LCD_PIN, OUTPUT);

  Display.begin();
  Touch.begin();
  Display.setRotation(1);
  Touch.setRotation(1);

  analogWrite(LCD_PIN, 255);
  Display.fillScreen(C_BLACK);

  Speed.init (20, 60, 160, 100,  5000, 0, 50, C_WHITE, C_BLACK, C_ORANGE);
  Accel.init (20, 110, 160, 1, 20, 0, 1, C_WHITE, C_BLACK, C_ORANGE);
  Turns.init (20, 160, 160, 1,   100, 0,  5, C_WHITE, C_BLACK, C_ORANGE);

  Direction.init(C_WHITE, C_ORANGE, C_DKGREY, C_WHITE, C_BLACK, 20, -2, Arial_14 );
  Button1.init(270, 40, 100, 40, C_WHITE, C_GREEN, C_BLACK, C_WHITE, "Run", -10, -5, Arial_14 ) ;
  Button2.init(270, 40, 100, 40, C_WHITE, C_RED, C_BLACK, C_WHITE,  "Stop", -10, -8, Arial_14 ) ;
  Button2.disable();

  Lock.init(20, 210, C_WHITE, C_ORANGE, C_GREY, C_WHITE, C_BLACK, 20, 5, "Keep locked", Arial_14 );

  OptionA0ID = Direction.add(20, 180, "CW");
  OptionA1ID = Direction.add(80, 180, "CCW");

  Resolution.init(C_WHITE, C_ORANGE, C_DKGREY, C_WHITE, C_BLACK, 20, -2, Arial_14 );
  OptionR0ID = Resolution.add(240,  80, "Full", SR01);
  OptionR1ID = Resolution.add(240, 110, "1/2", SR02);
  OptionR2ID = Resolution.add(240, 140, "1/4", SR04);
  OptionR3ID = Resolution.add(240, 170, "1/8", SR08);
  OptionR4ID = Resolution.add(240, 200, "1/16", SR16);

  Nema17.begin(400, 4);

  Nema17.setStepResolution(SR16);
  Nema17.setStepsPerRev(200); // 20 for DVD motor

  Display.setFont(Arial_14);
  Display.setTextColor(C_WHITE, C_BLACK);

  Display.setCursor(10, 30);
  sprintf(buf, "Speed: %ld", SpeedVal);
  SpeedText.print(buf);
  Speed.draw(SpeedVal);

  Display.setCursor(10, 80);
  sprintf(buf, "Acceleration: %ld", AccelVal);
  AccelText.print(buf);
  Accel.draw(AccelVal);

  Display.setCursor(10, 130);
  sprintf(buf, "Turns: %ld", TurnsVal);
  TurnText.print(buf);
  Turns.draw(TurnsVal);

  Direction.draw(dir);
  Button1.draw();
  Lock.draw(lock);
  Resolution.draw(resolution);

}

void loop() {

  if (Touch.touched()) {
    
    ProcessTouch();

    Speed.slide(BtnX, BtnY);
    Accel.slide(BtnX, BtnY);
    Turns.slide(BtnX, BtnY);
    Direction.press(BtnX, BtnY);
    Lock.press(BtnX, BtnY);
    Resolution.press(BtnX, BtnY);
   
    SpeedVal = Speed.value;
    AccelVal = Accel.value;
    TurnsVal = Turns.value;
    resolution = Resolution.value;
    lock = Lock.value;

    Display.setCursor(10, 30);
    sprintf(buf, "Speed: %ld", SpeedVal);
    SpeedText.print(buf);
    Speed.draw(SpeedVal);

    Display.setCursor(10, 80);
    sprintf(buf, "Acceleration: %ld", AccelVal);
    AccelText.print(buf);
    Accel.draw(AccelVal);

    Display.setCursor(10, 130);
    sprintf(buf, "Turns: %ld", TurnsVal);
    TurnText.print(buf);
    Turns.draw(TurnsVal);

    Nema17.setSpeed(SpeedVal, AccelVal);
    Nema17.setStepResolution(resolution);
    Nema17.setLock(lock);

    if (Direction.value) {
      dir = CW;
    }
    else {
      dir = CCW;
    }
    if (ProcessButtonPress(Button1)) {

      if (dir) {
        Nema17.moveRevolutions(Turns.value);
      }
      else {
        Nema17.moveRevolutions(-Turns.value);
      }

      Button1.disable();
      Button2.enable();
      Button2.draw();

    }
  }

  tt = 0;
  while (Nema17.IsRunning) {
    DrawBTN = true;
    if (tt > 50) {
      tt = 0;
      Serial.print("Motor RPM "); Serial.println(Nema17.getRPM());
      Serial.print("Revolutions "); Serial.println(Nema17.getRevolutions());
      Serial.print("Steps "); Serial.println(Nema17.getAbsoluteSteps());
      if (Touch.touched()) {
        ProcessTouch();
        if (ProcessButtonPress(Button2)) {
          Nema17.ForceStop();
          Button1.enable();
          Button2.disable();
          Button1.draw();
          break;
        }
      }
    }
  }

  if (DrawBTN) {
    DrawBTN = false;
    Button1.enable();
    Button2.disable();
    Button1.draw();
  }
}

void ProcessTouch() {

  // depending on the touch library you may need to change methods here

  p = Touch.getPoint();
  BtnX = p.x;
  BtnY = p.y;

  // consistency between displays is a mess...
  // this is some debug code to help show
  // where you pressed and the resulting map
  /*
    Serial.print("real coordinates: ");
    Serial.print(BtnX);
    Serial.print(",");
    Serial.print(BtnY);
    Display.drawPixel(BtnX, BtnY, C_RED);
  */

  //different values depending on where touch happened

  // x  = map(x, real left, real right, 0, 480);
  // y  = map(y, real bottom, real top, 320, 0);

  // tft with yellow headers
  //BtnX  = map(BtnX, 240, 0, 320, 0);
  //BtnY  = map(BtnY, 379, 0, 240, 0);

  // tft with black headers
  BtnX  = map(BtnX, 3800, 300, 0, 320);
  BtnY  = map(BtnY, 3800, 250, 0, 240);

  /*
    Serial.print(", Mapped coordinates: ");
    Serial.print(BtnX);
    Serial.print(",");
    Serial.println(BtnY);
    Display.drawPixel(BtnX, BtnY, C_GREEN);
    delay(50);
  */

}

bool ProcessButtonPress(Button TheButton) {

  if (TheButton.press(BtnX, BtnY)) {
    TheButton.draw(B_PRESSED);
    while (Touch.touched()) {
      ProcessTouch();
      Serial.print("ButtonPress: ");
      Serial.print(BtnX);
      Serial.print(",");
      Serial.println(BtnY);
      if (TheButton.press(BtnX, BtnY)) {
        TheButton.draw(B_PRESSED);
      }
      else {
        TheButton.draw(B_RELEASED);
        return false;
      }
      ProcessTouch();
    }

    TheButton.draw(B_RELEASED);
    return true;
  }
  return false;
}
