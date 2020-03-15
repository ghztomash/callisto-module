#include <Audio.h>
#include "callisto_hal.h"

// Audio Objects
AudioSynthWaveformSine			osc1;
AudioSynthWaveform				osc2;
AudioAnalyzeRMS					rms1;
AudioOutputAnalog				out1;
AudioConnection					patchCordrms(osc2, 0, rms1, 0);
AudioConnection					patchCordout(osc1, 0, out1, 0);

CallistoHAL callisto;

float amplitude[] = {0.0, 0.5, 0.9};

void setup(){
	callisto.setModeACallback(modeAChanged);
	callisto.setModeBCallback(modeBChanged);
	callisto.setTriggerCallback(trigger);
	callisto.setLED(UI_A,HIGH);
	callisto.setLED(UI_E,HIGH);
	
	AudioMemory(64);
	osc1.frequency(440);
	osc1.amplitude(0.0);
	osc2.begin(1.0, 0.25, WAVEFORM_SINE);
}

void loop(){
	callisto.update();
	
	AudioNoInterrupts();
	//osc1.frequency(callisto.readPotPitch());
	//osc1.frequency(callisto.readCVPitch());
	osc1.frequency(callisto.readPitch());
	AudioInterrupts();
	
	/*
	Serial.print(callisto.readPotPitch());
	Serial.print("\t ");
	Serial.print(callisto.readPotVolt(UI_C));
	Serial.print("\t ");
	Serial.println(callisto.readPotNorm(UI_C));
	delay(10);
	*/
	
	if(rms1.available()){
		float rms = rms1.read();
		callisto.setTrigLED(rms * 255);
	}
}

void modeAChanged(int mode){
	Serial.print("Mode A changed: ");
	osc1.amplitude(amplitude[mode]);
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