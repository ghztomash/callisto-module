#include "callisto_hal.h"
CallistoHAL callisto;

void setup(){
	callisto.setModeACallback(modeAChanged);
	callisto.setModeBCallback(modeBChanged);
}

void loop(){
	callisto.update();
}

void modeAChanged(int mode){
	Serial.print("Mode A changed: ");
	Serial.println(mode);
}

void modeBChanged(int mode){
	Serial.print("Mode B changed: ");
	Serial.println(mode);
}