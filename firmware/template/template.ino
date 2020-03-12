#include "callisto_hal.h"
CallistoHAL callisto;

void setup(){
	callisto.setModeACallback(modeAChanged);
	callisto.setModeBCallback(modeBChanged);
	callisto.setTriggerCallback(trigger);
	callisto.setLED(cA,HIGH);
	callisto.setLED(cE,HIGH);
}

void loop(){
	callisto.update();
	callisto.setTrigLED(255);
	Serial.print(callisto.readCVNorm(cA));
	Serial.print("\t ");
	Serial.println(callisto.readCVRaw(cA));
	delay(10);
}

void modeAChanged(int mode){
	Serial.print("Mode A changed: ");
	Serial.println(mode);
}

void modeBChanged(int mode){
	Serial.print("Mode B changed: ");
	Serial.println(mode);
}

void trigger(){
	Serial.println("Trigger!");
}