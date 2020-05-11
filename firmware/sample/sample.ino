/* Callisto Eurorack Module - Template Firmware
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

#include <Audio.h>
#include "callisto_hal.h"
#include "play_memory_sample.h"

#include "AudioSampleSin30.h"
#include "AudioSampleSin100.h"
#include "AudioSampleSin440.h"

// Audio Objects
AudioSynthWaveformDc			dcMod1;
AudioPlayMemorySample			sample1;
AudioAnalyzeRMS					rms1;
AudioAmplifier					inverter;
AudioOutputAnalog				out1;
AudioConnection					patchCordrms4(dcMod1, 0, sample1, 0);
AudioConnection					patchCordrms3(sample1, 0, rms1, 0);
AudioConnection					patchCordout1(sample1, 0, inverter, 0);
AudioConnection					patchCordout2(inverter, 0, out1, 0);

CallistoHAL callisto;

void setup(){
	callisto.setModeCallback(MODE_A, modeAChanged);
	callisto.setModeCallback(MODE_B, modeBChanged);
	callisto.setButtonCallback(MODE_A, buttonA);
	callisto.setButtonCallback(MODE_B, buttonB);
	callisto.setTriggerCallback(trigger);
	
	AudioMemory(64);
	
	dcMod1.amplitude(1.0);
	
	sample1.setSpeed(0.1);
	sample1.setSample(AudioSampleSin100);
	sample1.frequencyModulation(0.0);
	
	inverter.gain(-0.9);
}

void loop(){
	callisto.update();
	
	//float sampleSpeed = pow(2.0, callisto.readCVVolt(UI_A));
	float sampleSpeed = constrain(pow(2.0, (callisto.readPotNorm(UI_C)-0.5)*4.0 + callisto.readCVVolt(UI_A)), 0.0, 8.0);
	
	/*
	Serial.print(callisto.readCVVolt(UI_A));
	Serial.print(" \t ");
	Serial.print(pow(2.0, callisto.readCVVolt(UI_A)));
	Serial.print(" \t ");
	
	
	Serial.print((callisto.readPotNorm(UI_C)-0.5)*4.0);
	Serial.print(" \t ");
	Serial.print(pow(2.0, (callisto.readPotNorm(UI_C)-0.5)*4.0));
	Serial.print(" \n");
	*/
	
	delay(50);
	
	AudioNoInterrupts();
	//sample1.setSpeed(sampleSpeed);
	AudioInterrupts();
	
	if(rms1.available()){
		float rms = rms1.read();
		callisto.setTrigLED(rms * 255);
	}
}

void modeAChanged(int mode){
	Serial.print("Mode A changed: ");
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
	bool triggerState = !digitalReadFast(TRIGIN_PIN); // override any library calls for faster response TODO compare response time
	
	if(triggerState == HIGH){
		
		AudioNoInterrupts();
		sample1.play();
		AudioInterrupts();
		
	}
}

// TODO Add HAL function prototypes here for reference.