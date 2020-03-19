/* Callisto Eurorack Module - Hardware Abstraction Layer
 * Copyright (c) 2019 Tomash Ghz
 *
 * Please support GHz Labs and open-source hardware by purchasing
 * products directly from GHz Labs, donating or becoming a Github Sponsor!
 * 
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

#define MODE_A 0
#define MODE_B 1

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
		void setPitchCVSource(int);
		void setPitchPotSource(int);
		void setLED(int, bool);
		void setTrigLED(int);
		bool readButton(int);
		bool readTrigger();
		int getMode(int);
		void setMode(int, int);
		void setAutoModeSwitch(bool);
		void setModeCallback(int,void (*)(int));
		void setButtonCallback(int, void (*)());
		void setTriggerCallback(void (*)());
		
	private:
		void introAnimation();
		void blink();
		void calibrate();
		void saveCalibration();
		void loadCallibration();
		float linToExp(float,float);
		void debug(const char*);
		
		int pitchCVSource = UI_A;
		int pitchPotSource = UI_C;
		
		int ledsStates[6] = {0};
		int trigLedState = 0;
		int adcVal[12] = {0}; // RAW ADC values
		int adcCalibration[12] = {0}; // calibration table;
		int currentMode[2] = {0};
		bool tempButton[2] = {LOW};
		bool modeButton[2] = {LOW};
		bool lastModeButton[2] = {LOW};
		bool triggerState = LOW;
		bool autoModeSwitch = true;
		
		void (*modeACallback)(int) = NULL;
		void (*modeBCallback)(int) = NULL;
		void (*buttonACallback)() = NULL;
		void (*buttonBCallback)() = NULL;
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
	
	if(autoModeSwitch){
		ledsStates[currentMode[0]+3] = HIGH;
		ledsStates[currentMode[1]] = HIGH;
	}
	
	// boot callibration mode
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
	
	tempButton[0] = !digitalRead(SWITCHA_PIN); // invert logic becase of pullup
	tempButton[1] = !digitalRead(SWITCHB_PIN); // invert logic becase of pullup
	triggerState = !digitalRead(TRIGIN_PIN); // invert logic becase of pullup
	
	int nextMode = 0;
	
	for(int i=0; i<2; i++){
		if(tempButton[i] != lastModeButton[i]){
			if(tempButton[i] == HIGH){
				modeButton[i] = tempButton[i];
				
				if(autoModeSwitch){
					nextMode = getMode(i)+1;
					if(nextMode >= 3)
						nextMode = 0;
					setMode(i, nextMode);
				}
			} else {
				modeButton[i] = LOW;
			}
			lastModeButton[i] = tempButton[i];
		}
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

void CallistoHAL::setPitchCVSource(int source){ // set which input the 1v/oct pitch is coming from
	if (source < 0)
		return;
	if (source > 5)
		return;
	pitchCVSource = source;
}

float CallistoHAL::readCVPitch(){ // convert 1v/oct input to frequency
	return FREQ_MID_C * pow(2.0, readCVVolt(pitchCVSource)-1.0);
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

int CallistoHAL::readPotRaw(int adc){ // read raw potentiometer ADC values
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

void CallistoHAL::setPitchPotSource(int source){ // set which input the 1v/oct pitch is coming from
	if (source < 0)
		return;
	if (source > 5)
		return;
	pitchPotSource = source;
}

float CallistoHAL::readPotPitch(){ // convert Potentiometer input to frequency
	return FREQ_MID_C * pow(2.0, readPotNorm(pitchPotSource)*2.0-3.0);
}

float CallistoHAL::readPitch(){ // convert 1v/oct + Potentiometer input to frequency
	return FREQ_MID_C * pow(2.0, readCVVolt(pitchCVSource)-1.0 + readPotNorm(pitchPotSource)*2.0-2.0);
}


void CallistoHAL::setModeCallback(int modeSelector, void (*callback)(int)){ // assign pointer to call back function
	if(callback==NULL)
		return;
	if (modeSelector==0)
		modeACallback = callback;
	else if (modeSelector==1)
		modeBCallback = callback;
}

void CallistoHAL::setTriggerCallback(void (*callback)()){ // assign trigger callback function
	if(callback==NULL)
		return;
	attachInterrupt(TRIGIN_PIN, callback, FALLING);
}

void CallistoHAL::setButtonCallback(int button, void (*callback)()){ // assign trigger callback function 
//TODO!: it's possible to pass a function with parameters, bad things will happen
	if(callback==NULL)
		return;
	if (button==0)
		attachInterrupt(SWITCHA_PIN, callback, FALLING);
	else if (button==1)
		attachInterrupt(SWITCHB_PIN, callback, FALLING);
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

void CallistoHAL::setMode(int modeSelector, int modeValue){ // manually update mode
	if(modeValue > 2)
		modeValue = 2;
	if(modeValue < 0)
		modeValue = 0;
	
	if(modeSelector > 1)
		modeSelector = 1;
	if(modeSelector < 0)
		modeSelector = 0;
	
	if(modeSelector==0)
		ledsStates[currentMode[modeSelector]+3] = LOW; // turn off last mode LED
	else 
		ledsStates[currentMode[modeSelector]] = LOW; // turn off last mode LED
	
	currentMode[modeSelector] = modeValue; // update mode value
	
	if(modeSelector==0){
		ledsStates[currentMode[modeSelector]+3] = HIGH; // set new LED
		if (modeACallback != NULL){
			modeACallback(currentMode[modeSelector]); // call callback function
		}
	} else if(modeSelector==1){
		ledsStates[currentMode[modeSelector]] = HIGH; // set new LED
		if (modeBCallback != NULL){
			modeBCallback(currentMode[modeSelector]); // call callback function
		}
	}
}

int CallistoHAL::getMode(int modeSelector){ // return which mode A / B is in
	if ((modeSelector == 0) || (modeSelector == 1))
		return currentMode[modeSelector];
	return -1;
}

void CallistoHAL::setAutoModeSwitch(bool switchEnabled){ // enable automatic mode switching on button press
	autoModeSwitch = switchEnabled;
}

bool CallistoHAL::readButton(int button){ // return button state
	if ((button == 0) || (button == 1))
		return modeButton[button];
	return false;
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

				adcOffset[i] = offset;
				
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
	//TODO: Check for Version Number
		debug("Eeprom data loaded");
		for (int i=0; i<12; i++ ){
			adcCalibration[i] = eedata.calibration[i];
		}
	} else {
		debug("No Eeprom data");
	}
}

void CallistoHAL::debug(const char str[]){
	#ifdef DEBUG
		Serial.println(str);
	#endif
}

#endif