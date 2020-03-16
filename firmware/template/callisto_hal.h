#ifndef CallistoHAL_h
#define CallistoHAL_h

#include <Arduino.h>
#include <EEPROM.h>

#define VERSION 1
#define DEBUG 1

// PIN defines
#define LEDA_PIN 0
#define LEDB_PIN 1
#define LEDC_PIN 2
#define LEDD_PIN 3
#define LEDE_PIN 4
#define LEDF_PIN 5
#define LEDTRIG_PIN 10
#define SWITCHA_PIN 8
#define SWITCHB_PIN 9
#define TRIGIN_PIN 11

#define CVA_PIN  A0
#define POTA_PIN A1
#define CVB_PIN  A2
#define POTB_PIN A3
#define CVC_PIN  A4
#define POTC_PIN A5
#define CVD_PIN  A6
#define POTD_PIN A7
#define CVE_PIN  A8
#define POTE_PIN A9
#define CVF_PIN  A10
#define POTF_PIN A11

// Array Defines
#define CVA  0
#define POTA 1
#define CVB  2
#define POTB 3
#define CVC  4
#define POTC 5
#define CVD  6
#define POTD 7
#define CVE  8
#define POTE 9
#define CVF  10
#define POTF 11

// UI Defines
#define UI_A 0
#define UI_B 1
#define UI_C 2
#define UI_D 3
#define UI_E 4
#define UI_F 5

#define FREQ_MID_C 261.625565

#define ADC_RESOLUTION 12
#define ADC_AVERAGING 1
int MAXADC = (1 << ADC_RESOLUTION) - 1;	// maximum possible reading from ADC

int adcPins[12] = {CVA_PIN, POTA_PIN, CVB_PIN, POTB_PIN, CVC_PIN, POTC_PIN, CVD_PIN, POTD_PIN, CVE_PIN, POTE_PIN, CVF_PIN, POTF_PIN};
int ledsPins[6] = {LEDA_PIN, LEDB_PIN, LEDC_PIN, LEDD_PIN, LEDE_PIN, LEDF_PIN};

struct EepromData {
	int config;
	int version;
	int calibration[12];
};

class CallistoHAL{
	public:
		CallistoHAL();
		void update();
		void updateADC();
		void updateButtons();
		void updateLEDs();
		int readADCRaw(int);
		int readCVRaw(int);
		float readCVNorm(int);
		float readCVVolt(int);
		float readCVPitch();
		int readPotRaw(int);
		float readPotNorm(int);
		float readPotVolt(int);
		float readPotPitch();
		float readPitch();
		void setLED(int, bool);
		void setTrigLED(int);
		bool readButton(int);
		bool readTrigger();
		int getMode(int);
		void setModeA(int);
		void setModeB(int);
		void setModeACallback(void (*)(int));
		void setModeBCallback(void (*)(int));
		void setTriggerCallback(void (*)());
		
	private:
		void introAnimation();
		void blink();
		void calibrate();
		void saveCalibration();
		void loadCallibration();
		float linToExp(float,float);
		void debug(const char*);
		
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
		bool triggerState = LOW;
		
		void (*modeACallback)(int) = NULL;
		void (*modeBCallback)(int) = NULL;
};

CallistoHAL::CallistoHAL(){
	
	//Setup GPIOs
	for (int i=0; i<6; i++) {
		pinMode(ledsPins[i], OUTPUT);
	}
	pinMode(LEDTRIG_PIN, OUTPUT);
	pinMode(TRIGIN_PIN, INPUT_PULLUP);
	pinMode(SWITCHA_PIN, INPUT_PULLUP);
	pinMode(SWITCHB_PIN, INPUT_PULLUP);
	
	//Setup ADC
	analogReference(EXTERNAL);
	analogReadResolution(ADC_RESOLUTION);
	analogReadAveraging(ADC_AVERAGING);
	
	//setup PWM for LED
	analogWriteResolution(8);
	analogWriteFrequency(LEDTRIG_PIN, 187500);
	
	loadCallibration();
	
	introAnimation();
	
	ledsStates[currentModeA+3] = HIGH;
	ledsStates[currentModeB] = HIGH;
	
	// boot callibration
	if ((digitalRead(SWITCHA_PIN) == LOW) && (digitalRead(SWITCHB_PIN) == LOW)){
		calibrate();
	}
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
	digitalWrite(LEDTRIG_PIN, HIGH);
	delay(100);
	
}

void CallistoHAL::update(){
	updateADC();
	updateButtons();
	updateLEDs();
}

void CallistoHAL::updateADC(){
	for (int i=0; i<12; i++ ){
		adcVal[i] = analogRead(adcPins[i]) - adcCalibration[i]; // callibration offset
	}
}

void CallistoHAL::updateButtons(){
	
	tempButtonA = digitalRead(SWITCHA_PIN);
	tempButtonB = digitalRead(SWITCHB_PIN);
	triggerState = !digitalRead(TRIGIN_PIN); // invert logic becase of pullup
	
	if(tempButtonA != lastModeButtonA){
		if(tempButtonA == LOW){
			modeButtonA = HIGH; // invert logic becase of pullup
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
			modeButtonB = HIGH; // invert logic becase of pullup
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
}

void CallistoHAL::updateLEDs(){ // update LEDs to their new values
	for (int i=0; i<6; i++) {
		digitalWrite(ledsPins[i], ledsStates[i]);
	}
	analogWrite(LEDTRIG_PIN, trigLedState);
}

int CallistoHAL::readADCRaw(int adc){
	if (adc < 0)
		adc = 0;
	if (adc > 11)
		adc = 11;
	return adcVal[adc];
}

int CallistoHAL::readCVRaw(int adc){
	if (adc < 0)
		adc = 0;
	if (adc > 5)
		adc = 5;
	return (MAXADC - adcVal[adc*2]); //invert value because of inverting opamp
}

float CallistoHAL::readCVNorm(int adc){ // read CV normalized from -1.0 to 1.0
	if (adc < 0)
		adc = 0;
	if (adc > 5)
		adc = 5;
	if (adc == 0)
		return (float)adcVal[adc*2]/MAXADC*-2.0+1.43; // 1v/oct input is a bit offset in this case 5V is 1.0 but it can go up to 7V
	return (float)adcVal[adc*2]/MAXADC*-2.0+1; // invert value because of inverting opamp
}

float CallistoHAL::readCVPitch(){ // convert 1v/oct input to frequency
	return FREQ_MID_C * pow(2.0, readCVVolt(UI_A)-1.0);
}

float CallistoHAL::readCVVolt(int adc){ // read CV Voltage
	if (adc < 0)
		adc = 0;
	if (adc > 5)
		adc = 5;
	if (adc == 0)
		return ((float)adcVal[adc*2]/(float)MAXADC*3.3)/-0.33+7.14; // 1v/oct input is a bit offset
	return ((float)adcVal[adc*2]/(float)MAXADC*3.3)/-0.33+5; // invert value because of inverting opamp
}

int CallistoHAL::readPotRaw(int adc){
	if (adc < 0)
		adc = 0;
	if (adc > 5)
		adc = 5;
	return adcVal[adc*2+1];
}

float CallistoHAL::readPotNorm(int adc){ // read potentiometer normalized from 0 to 1.0
	if (adc < 0)
		adc = 0;
	if (adc > 5)
		adc = 5;
	return (float)adcVal[adc*2+1]/MAXADC; //invert value because of inverting opamp
}

float CallistoHAL::readPotVolt(int adc){ // read potentiometer voltage
	if (adc < 0)
		adc = 0;
	if (adc > 5)
		adc = 5;
	return (float)adcVal[adc*2+1]/(float)MAXADC*3.3; //invert value because of inverting opamp
}

float CallistoHAL::readPotPitch(){ // convert Potentiometer input to frequency
	return FREQ_MID_C * pow(2.0, readPotNorm(UI_C)*2.0-3.0);
}

float CallistoHAL::readPitch(){ // convert 1v/oct + Potentiometer input to frequency
	return FREQ_MID_C * pow(2.0, readCVVolt(UI_A)-1.0 + readPotNorm(UI_C)*2.0-3.0);
}


void CallistoHAL::setModeACallback(void (*callback)(int)){ // assign pointer to call back function
	if(callback!=NULL)
		modeACallback = callback;
}

void CallistoHAL::setModeBCallback(void (*callback)(int)){ // assign pointer to call back function
	if(callback!=NULL)
		modeBCallback = callback;
}

void CallistoHAL::setTriggerCallback(void (*callback)()){ // assign trigger callback function
	if(callback!=NULL)
		attachInterrupt(TRIGIN_PIN, callback, FALLING);
}

void CallistoHAL::setLED(int led, bool state){ // manually change LED state
	if (led < 0)
		led = 0;
	if (led > 5)
		led = 5;
	ledsStates[led] = state;
}

void CallistoHAL::setTrigLED(int val){ // set PWM value for trigger LED
	if (val < 0)
		val = 0;
	if (val > 255)
		val = 255; // depends on PWM resolutin!!!
	trigLedState = val;
}

void CallistoHAL::setModeA(int mode){ // manually update mode
	ledsStates[currentModeA+3] = LOW;
	if(mode >= 3)
		mode = 2;
	if(mode < 0)
		mode = 0;
	currentModeA = mode;
	ledsStates[currentModeA+3] = HIGH;
	
	if (modeACallback != NULL){
		modeACallback(currentModeA); // call callback function
	}
}

void CallistoHAL::setModeB(int mode){ // manually update mode
	ledsStates[currentModeB] = LOW;
	if(mode >= 3)
		mode = 2;
	if(mode < 0)
		mode = 0;
	currentModeB = mode;
	ledsStates[currentModeB] = HIGH;
	
	if (modeBCallback != NULL){
		modeBCallback(currentModeB); // call callback function
	}
}

int CallistoHAL::getMode(int mode){ // return which mode A / B is in
	if (mode == 0)
		return currentModeA;
	if (mode == 1)
		return currentModeB;
	return -1;
}

bool CallistoHAL::readButton(int button){ // return button state
	if (button == 0)
		return modeButtonA;
	else if (button == 1)
		return modeButtonB;
	return -1;
}

bool CallistoHAL::readTrigger(){ // return trigger state
	return triggerState;
}

void CallistoHAL::blink(){
	for (int i=0; i<6; i++) {
		digitalWrite(ledsPins[i], HIGH);
	}
	digitalWrite(LEDTRIG_PIN, HIGH);
	delay(100);
	for (int i=0; i<6; i++) {
		digitalWrite(ledsPins[i], LOW);
	}
	digitalWrite(LEDTRIG_PIN, LOW);
	delay(100);
}

void CallistoHAL::calibrate(){ // callibrate ADC input offset. Disconnect any CV inputs before running.
	debug("Calibration mode");
	
	for (int i=0; i<12; i++ ){
		adcCalibration[i] = 0;
	}
	
	int adcOffset[12] = {0}; // offset table;
	int expected = MAXADC*(1.65/3.3);	// expected ADC reading
	int expectedVoct = MAXADC*(2.357/3.3);	// expected ADC reading
	
	while(true){
		blink();
		if ((digitalRead(SWITCHA_PIN) == HIGH) && (digitalRead(SWITCHB_PIN) == HIGH)) {
			debug("Run Calibration");
			
			delay(500);
			for (int i=0; i<12; i++ ){
				delay(5);	  
				long sum = 0;
				int offset;
				float average;
	  
				for (int j=0; j<5000; j++) {
					sum += analogRead(adcPins[i]); // callibration offset
				}
				average = (float)sum/(float)5000;

				if(i==0) // for 1v/oct input
					offset = average - expectedVoct;
				else
					offset = average - expected;

				adcOffset[i] = sOffset;
				
			}
			break;
		}
	}
	delay(100);
	
	for (int i=0; i<12; i++ ){
		if(i%2==0){
			adcCalibration[i] = adcOffset[i];
		}
	}
	saveCalibration();
	debug("Calibration Complete!");
	delay(1000);
	blink();
	//_reboot_Teensyduino_(); // Doesn't work if module is not connected through USB
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
		debug("Eeprom data loaded");
		for (int i=0; i<12; i++ ){
			adcCalibration[i] = eedata.calibration[i];
		}
	} else {
		debug("No Eeprom data");
	}
}

float CallistoHAL::linToExp(float index, float range){
  //convert linear values to exponential
  float ratio = pow(range, 1/(float)range);
  return pow(ratio,index);
}

void CallistoHAL::debug(const char str[]){
	#ifdef DEBUG
		Serial.println(str);
	#endif
}

#endif