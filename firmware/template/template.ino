#include "callisto_hal.h"
CallistoHAL callisto;

void setup(){
	callisto.setModeACallback(modeAChanged);
	callisto.setModeBCallback(modeBChanged);
	callisto.setTriggerCallback(trigger);
	callisto.setLED(UI_A,HIGH);
	callisto.setLED(UI_E,HIGH);
}

void loop(){
	callisto.update();
	callisto.setTrigLED(255);
	//Serial.print(callisto.readCVVolt(UI_A));
	//Serial.print("\t ");
	//Serial.println(callisto.readCVNorm(UI_A));
	//delay(10);
}

void modeAChanged(int mode){
	Serial.print("Mode A changed: ");
	Serial.println(mode);
}

void modeBChanged(int mode){
	Serial.print("Mode B changed: ");
	Serial.println(mode);
	
	Serial.print(callisto.readCVVolt(UI_A));
	Serial.print("\t ");
	Serial.println(callisto.readCVNorm(UI_A));
}

void trigger(){
	Serial.println("Trigger!");
}