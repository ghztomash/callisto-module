#ifndef CallistoHAL_h
#define CallistoHAL_h

#include <Arduino.h>
#include <EEPROM.h>

#define VERSION 1
#define DEBUG 1

// PIN defines
#define LEDA 0
#define LEDB 1
#define LEDC 2
#define LEDD 3
#define LEDE 4
#define LEDF 5
#define LEDTRIG 10

#define SWITCHA 8
#define SWITCHB 9

#define TRIGIN 11

#define CVA  A0
#define POTA A1
#define CVB  A2
#define POTB A3
#define CVC  A4
#define POTC A5
#define CVD  A6
#define POTD A7
#define CVE  A8
#define POTE A9
#define CVF  A10
#define POTF A11

#define ADC_RESOLUTION 13
#define ADC_AVERAGING 8

int adcPins[12] = {CVA, POTA, CVB, POTB, CVC, POTC, CVD, POTD, CVE, POTE, CVF, POTF};
int ledsPins[6] = {LEDA, LEDB, LEDC, LEDD, LEDE, LEDF};

struct EepromData {
	int config;
	int version;
	int calibration[12];
};

class CallistoHAL{
	public:
		CallistoHAL();
		void blink();
		void introAnimation();
		void update();
		void updateADC();
		void updateUI();
		int readADCRaw(int);
		int readCVRaw(int);
		float readCVnorm(int);
		float readCVvolt(int);
		float readCVPitch();
		int readPotRaw(int);
		float readPotNorm(int);
		float readPotVolt(int);
		float readPotPitch();
		void setLED(int);
		void setTrigLED(int);
		int readButton(int);
		int readTrigger();
		int getModeA();
		int getModeB();
		void setModeA(int);
		void setModeB(int);
		
		void setModeACallback(void (*)(int));
		void setModeBCallback(void (*)(int));
	private:
		void saveCalibration();
		void loadCallibration();
		float linToExp(float,float);
		
		int ledsStates[6] = {0};
		int trigLedState = 0;
		int adcVal[12] = {0}; // RAW ADC values
		int adcCalibration[12] = {0}; // calibration table;
		int currentModeA = 0;
		int currentModeB = 0;
		bool tempButtonA = LOW;
		bool tempButtonB = LOW;
		bool modeButtonA = LOW;
		bool lastModeButtonA = LOW;
		bool modeButtonB = LOW;
		bool lastModeButtonB = LOW;
		
		void (*modeACallback)(int) = NULL;
		void (*modeBCallback)(int) = NULL;
		void (*triggerCallback)(int) = NULL;
};

CallistoHAL::CallistoHAL(){
	
	//Setup GPIOs
	for (int i=0; i<6; i++) {
		pinMode(ledsPins[i], OUTPUT);
	}
	pinMode(LEDTRIG, OUTPUT);
	pinMode(TRIGIN, INPUT_PULLUP);
	pinMode(SWITCHA, INPUT_PULLUP);
	pinMode(SWITCHB, INPUT_PULLUP);
	
	//Setup ADC
	analogReference(EXTERNAL);
	analogReadResolution(ADC_RESOLUTION);
	analogReadAveraging(ADC_AVERAGING);
	
	loadCallibration();
	
	introAnimation();
	
	ledsStates[currentModeA+3] = HIGH;
	ledsStates[currentModeB] = HIGH;
}

void CallistoHAL::blink(){
	digitalWrite(LEDTRIG, HIGH);
	delay(200);
	digitalWrite(LEDTRIG, LOW);
	delay(200);
}

void CallistoHAL::introAnimation(){
	for (int i=0; i<6; i++) {
		digitalWrite(ledsPins[i], HIGH);
		delay(100);
	}
	for (int i=0; i<6; i++) {
		digitalWrite(ledsPins[i], LOW);
		delay(100);
	}
	digitalWrite(LEDTRIG, HIGH);
	delay(100);
}

void CallistoHAL::update(){
	updateADC();
	updateUI();
}

void CallistoHAL::updateADC(){
	for (int i=0; i<12; i++ ){
		adcVal[i] = analogRead(adcPins[i]) - adcCalibration[i]; // callibration offset
	}
}

void CallistoHAL::updateUI(){
	
	tempButtonA = digitalRead(SWITCHA);
	tempButtonB = digitalRead(SWITCHB);
	
	if(tempButtonA != lastModeButtonA){
		if(tempButtonA == LOW){
			modeButtonA = HIGH;
			ledsStates[currentModeA+3] = LOW;
			currentModeA++;
			if(currentModeA >= 3)
				currentModeA = 0;
			ledsStates[currentModeA+3] = HIGH;
			if (modeACallback != NULL){
				modeACallback(currentModeA); // call callback function
			}
		} else {
			modeButtonA = LOW;
		}
		lastModeButtonA = tempButtonA;
	}
	
	if(tempButtonB != lastModeButtonB){
		if(tempButtonB == LOW){
			modeButtonB = HIGH;
			ledsStates[currentModeB] = LOW;
			currentModeB++;
			if(currentModeB >= 3)
				currentModeB = 0;
			ledsStates[currentModeB] = HIGH;
			if (modeBCallback != NULL){
				modeBCallback(currentModeB); // call callback function
			}
		} else {
			modeButtonB = LOW;
		}
		lastModeButtonB = tempButtonB;
	}
	
	for (int i=0; i<6; i++) {
		digitalWrite(ledsPins[i], ledsStates[i]);
	}
	analogWrite(LEDTRIG, trigLedState);
}

void CallistoHAL::setModeACallback(void (*callback)(int)){ // assign pointer to call back function
	if(callback!=NULL)
		modeACallback = callback;
}

void CallistoHAL::setModeBCallback(void (*callback)(int)){ // assign pointer to call back function
	if(callback!=NULL)
		modeBCallback = callback;
}

void CallistoHAL::saveCalibration(){
	int eeAddress = 0;
	EepromData eedata = { 0xDEAD, VERSION, {0}};
	
	for (int i=0; i<12; i++ ){
		eedata.calibration[i] = adcCalibration[i];
	}
	
	EEPROM.put(eeAddress, eedata);
}

void CallistoHAL::loadCallibration(){
	int eeAddress = 0;
	EepromData eedata;
	EEPROM.get(eeAddress, eedata);
	
	if(eedata.config == 0xDEAD){
		#ifdef DEBUG
		Serial.println("Eeprom data loaded");
		#endif
		for (int i=0; i<12; i++ ){
			adcCalibration[i] = eedata.calibration[i];
		}
	} else {
		#ifdef DEBUG
		Serial.println("No Eeprom data");
		#endif
	}
}

float CallistoHAL::linToExp(float index, float range){
  //convert linear values to exponential
  float ratio = pow(range, 1/(float)range);
  return pow(ratio,index);
}


#endif