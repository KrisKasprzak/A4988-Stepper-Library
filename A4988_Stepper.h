
#ifndef _A4988_STEPPER_h_
#define _A4988_STEPPER_h_

#define DUTY_ON 127
#define DUTY_OFF 0

#define CW 0
#define CCW 1

#define SR01 1
#define SR02 2
#define SR04 4
#define SR08 8
#define SR16 16

#include "Arduino.h"


class A4988_Stepper {
	
public:

	A4988_Stepper(uint8_t StepPin, uint8_t DirPin, uint8_t EnPin, uint8_t FeedbackPin, uint8_t MS1Pin, uint8_t MS2Pin,uint8_t MS3Pin);
	A4988_Stepper(uint8_t StepPin, uint8_t DirPin, uint8_t EnPin, uint8_t FeedbackPin);

	void begin(uint32_t MaxSpeed, uint32_t Acceleration );
	void setSpeed(uint32_t MaxSpeed, uint32_t Acceleration );
	void getSpeed(uint32_t &MaxSpeed, uint32_t &Acceleration );
	void moveSteps(long Steps);
	void moveRevolutions(float Revolutions);
	
	float getRevolutions();
	
	bool Spin(uint32_t Revolutions);
	
	void setStepResolution(uint8_t Value);
	uint8_t getStepResolution();
	uint16_t getRPM();
	uint32_t getSteps();
	long getAbsoluteSteps();
	uint32_t getPWMFrequency();
	void setLock(bool Value);

	void Stop();
	void ForceStop();
		
	uint8_t getStepsPerRev();
	void setStepsPerRev(uint8_t Value);
	
	bool IsRunning = false;
	
	volatile long Steps = 0, CurrentSteps = 0, AbsoluteSteps = 0;

	volatile long W = 0;
	uint8_t StepResolution = SR01;
	bool lock = false;
	uint8_t Direction = CCW;

	
	
protected:

private:

    uint8_t StepsPerRev = 200;
	uint8_t pinStep = 0;
	
	uint8_t pinMS1= 0;
	uint8_t pinMS2= 0;
	uint8_t pinMS3= 0;
	uint8_t pinDir= 0;
	uint8_t pinEn = 0;
	uint8_t pinFB = 0;
};	


#endif
