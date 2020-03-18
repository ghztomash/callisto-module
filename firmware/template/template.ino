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

float amplitude[] = {0.9, 0.5, 0.0};

void setup(){
	callisto.setModeCallback(MODE_A, modeAChanged);
	callisto.setModeCallback(MODE_B, modeBChanged);
	callisto.setButtonCallback(MODE_A, buttonA);
	callisto.setButtonCallback(MODE_B, buttonB);
	callisto.setTriggerCallback(trigger);
	
	AudioMemory(64);
	osc1.frequency(440);
	osc1.amplitude(0.9);
	osc2.begin(1.0, 0.25, WAVEFORM_SINE);
}

void loop(){
	callisto.update();
	
	AudioNoInterrupts();
	osc1.frequency(callisto.readPitch());
	AudioInterrupts();
	
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
}

void buttonA(){
	Serial.println("Button A pressed.");
}

void buttonB(){
	Serial.println("Button B pressed.");
}

void trigger(){
	Serial.println("Trigger!");
}