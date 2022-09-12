
#include "A4988_Stepper.h"


static A4988_Stepper *isrPointer;
void PulseHandler(void);
	volatile unsigned long MaxA = 1000;
	volatile unsigned long MaxS = 1000;
	volatile unsigned long MaxFrequency = 0;
	volatile unsigned long PWMFrequency = 0;
	
uint8_t ps = 0, pe = 0;
float rev;

volatile long TempSteps = 0;

A4988_Stepper *o = isrPointer;

A4988_Stepper::A4988_Stepper(uint8_t StepPin, uint8_t DirPin, uint8_t EnPin, uint8_t FeedbackPin, uint8_t MS1Pin, uint8_t MS2Pin,uint8_t MS3Pin){
	
		pinStep = StepPin;
		pinDir = DirPin;
		pinEn = EnPin;
		pinFB = FeedbackPin;
		pinMS1 = MS1Pin;
		pinMS2 = MS2Pin;
		pinMS3 = MS3Pin;	

		pinMode(pinStep, OUTPUT);
		pinMode(pinDir, OUTPUT);
		pinMode(pinEn, OUTPUT);

		pinMode(pinMS1, OUTPUT);
		pinMode(pinMS2, OUTPUT);
		pinMode(pinMS3, OUTPUT);

		digitalWrite(pinMS1, LOW);
		digitalWrite(pinMS2, LOW);
		digitalWrite(pinMS3, LOW);

		StepResolution = SR01;
		
		StepsPerRev = 200;
		lock = false;
		pinMode(pinFB, INPUT);
		attachInterrupt(digitalPinToInterrupt(pinFB), PulseHandler, FALLING);
		
		isrPointer = this;
	  
}
	
A4988_Stepper::A4988_Stepper(uint8_t StepPin, uint8_t DirPin, uint8_t EnPin, uint8_t FeedbackPin){
	
		pinStep = StepPin;
		pinDir = DirPin;
		pinEn = EnPin;
		pinFB = FeedbackPin;

		pinMode(pinStep, OUTPUT);
		pinMode(pinDir, OUTPUT);
		pinMode(pinEn, OUTPUT);

		StepResolution = SR01;
		lock = false;
		pinMode(pinFB, INPUT);
		attachInterrupt(digitalPinToInterrupt(pinFB), PulseHandler, FALLING);
		
		isrPointer = this;
	  
}
	
void A4988_Stepper::begin(uint32_t MaxSpeed, uint32_t Acceleration){
	
	MaxS = MaxSpeed;
	MaxA = Acceleration;
	ps = pinStep;
	pe = pinEn;

	digitalWrite(pinEn, HIGH);
		
}



float A4988_Stepper::getRevolutions(){


	if (Direction == CCW) {
	  rev = (float) ((float)CurrentSteps / (float) (StepResolution * StepsPerRev));	
	}
	else {
	  rev = (float) ((float)-CurrentSteps / (float) (StepResolution * StepsPerRev));	
	}

	return rev;
	
}

void A4988_Stepper::setLock(bool Value){
	
	lock = Value;
	if (lock){
		digitalWrite(pinEn, LOW);
	}
	else {
		digitalWrite(pinEn, HIGH);
	}
	
}

uint8_t A4988_Stepper::getStepsPerRev(){
	
	return StepsPerRev;	
	
}

void A4988_Stepper::setStepsPerRev(uint8_t Value){
	
	StepsPerRev = Value;	
	
}

void A4988_Stepper::moveSteps(long Value) {
	
	digitalWriteFast(pinEn, LOW);
	
	W = DUTY_ON;
	CurrentSteps = 0;
	PWMFrequency = 0;
	MaxFrequency = 0;
	TempSteps = 0;
		  
	Steps = Value;

	if (Steps < 0) {
		Direction = CCW;
		Steps = abs(Steps);
		digitalWriteFast(pinDir, HIGH);
	}
	else if (Steps > 0) {
		Direction = CW;
		digitalWriteFast(pinDir, LOW);
	}

	IsRunning = true;

	analogWrite(pinStep, HIGH);

}

void A4988_Stepper::moveRevolutions(float Revolutions){

	Steps = Revolutions * StepResolution * StepsPerRev;

	moveSteps(Steps);
	
}

void A4988_Stepper::Stop() {

	// ramp down
	// force the ramp down part in the ISR
	CurrentSteps =  Steps - TempSteps;
		
}

void A4988_Stepper::ForceStop() {	
		
	Steps = 0;
	CurrentSteps = 0;
	PWMFrequency = 0;
	TempSteps = 0;
	W = DUTY_OFF;
	IsRunning = false;

	asm volatile("DSB");
	analogWriteFrequency(pinStep, PWMFrequency);
	asm volatile("DSB");
	analogWrite(pinStep, W);
	asm volatile("DSB");
	
	// shut the thing off
	digitalWrite(pinEn, HIGH);
	

}



void A4988_Stepper::setSpeed(uint32_t MaxSpeed, uint32_t Acceleration){
	
	MaxS = MaxSpeed;
	MaxA = Acceleration;
	
}

void A4988_Stepper::getSpeed(uint32_t &MaxSpeed, uint32_t &Acceleration){
	
	MaxSpeed = MaxS;
	Acceleration = MaxA;
	
}

uint8_t A4988_Stepper::getStepResolution(){
	
	return StepResolution;
	
}

void A4988_Stepper:: setStepResolution(uint8_t Value){
	
	StepResolution = Value;
	
	switch (StepResolution) {
		case SR01:
			digitalWrite(pinMS1, LOW);
			digitalWrite(pinMS2, LOW);
			digitalWrite(pinMS3, LOW);
			break;	
		case SR02:
			digitalWrite(pinMS1, HIGH);
			digitalWrite(pinMS2, LOW);
			digitalWrite(pinMS3, LOW);
			break;
		case SR04:
			digitalWrite(pinMS1, LOW);
			digitalWrite(pinMS2, HIGH);
			digitalWrite(pinMS3, LOW);
			break;
		case SR08:
			digitalWrite(pinMS1, HIGH);
			digitalWrite(pinMS2, HIGH);
			digitalWrite(pinMS3, LOW);
			break;
		case SR16:
			digitalWrite(pinMS1, HIGH);
			digitalWrite(pinMS2, HIGH);
			digitalWrite(pinMS3, HIGH);
			break;
	}
}


uint32_t A4988_Stepper::getPWMFrequency() {

	return PWMFrequency;

}


long A4988_Stepper::getAbsoluteSteps() {

	return AbsoluteSteps;

}

uint32_t A4988_Stepper::getSteps() {

	return CurrentSteps;


}


uint16_t A4988_Stepper::getRPM(){

	return (60.0 * PWMFrequency / (StepsPerRev * StepResolution));	
	
}

void PulseHandler() {
	
	
		
	if (o->Steps > 0) {

		o->W = DUTY_ON;

		o->CurrentSteps++;
		
		if (o->Direction == CCW) {
		  o->AbsoluteSteps++;
		}
		else {
		  o->AbsoluteSteps--;
		}

		// ramp up
		if (o->CurrentSteps <= (o->Steps / 2)) {
			
			PWMFrequency =  PWMFrequency + MaxA;
			
			if (o->getRPM() > MaxS) {
				PWMFrequency = MaxFrequency;
			}
			else {
				MaxFrequency = PWMFrequency;
				TempSteps++;
			}
		}
		// ramp down
		if ( o->CurrentSteps > (o->Steps - TempSteps)) {
			PWMFrequency = PWMFrequency - MaxA;
		}
		
		if (o->CurrentSteps >= (o->Steps)) {
			/*
		  o->Steps = 0;
		  o->CurrentSteps = 0;
		  
		  PWMFrequency = 0;
		  MaxFrequency = 0;
		  TempSteps = 0;
		  */
		  o->W = DUTY_OFF;
		  o->IsRunning = false;
		  		  
		  if (o->lock){
				digitalWrite(pe, LOW);
		  }
		  else {
				digitalWrite(pe, HIGH);
		  }
		}
	}
	  
  asm volatile("DSB");
  analogWriteFrequency(ps, PWMFrequency);
  asm volatile("DSB");
  analogWrite(ps, o->W);
  asm volatile("DSB");

}


