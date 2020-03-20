/* Callisto Eurorack Module - Drum Synth Firmware
 * Copyright (c) 2020 Tomash Ghz
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

// Audio Objects
AudioSynthWaveformModulated		osc1; // main oscillator
AudioEffectEnvelope				vca1; // main oscilator envelope
AudioAnalyzeRMS					rms1; // RMS analyzer for LED indicator
AudioMixer4						mix1;
AudioMixer4						mixMaster; // master mixer
AudioOutputAnalog				out1;

// Audio Object connections
AudioConnection					patchCordOsc1(osc1, 0, vca1, 0); // osc1 -> vca1
AudioConnection					patchCord4(vca1, 0, mix1, 0);
AudioConnection					patchCord6(mix1, 0, mixMaster, 0);
AudioConnection					patchCordoutlrms(mixMaster, 0, rms1, 0);
AudioConnection					patchCordout(mixMaster, 0, out1, 0);

CallistoHAL callisto;

float decay = 40;

void setup(){
	callisto.setModeCallback(MODE_A, modeAChanged);
	callisto.setModeCallback(MODE_B, modeBChanged);

	callisto.setTriggerChangeCallback(triggerChange);
	
	AudioMemory(64);
	osc1.begin(WAVEFORM_SINE);
	osc1.frequency(40);
	osc1.amplitude(1.0);
	osc1.frequencyModulation(0);
	
	vca1.attack(0);
	vca1.sustain(0.6);
	vca1.delay(0);
	vca1.hold(0);
	
	mix1.gain(0, 1.0);
	mixMaster.gain(0, 0.9);
}

void loop(){
	callisto.update();
	
	decay = callisto.readPotNorm(UI_D)*1000.0;
	
	AudioNoInterrupts();
	osc1.frequency(callisto.readPitch());
	vca1.release(decay);
	vca1.decay(decay);
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

void triggerChange(){
	bool triggerState = !digitalReadFast(TRIGIN_PIN); // override any library calls for faster response TODO compare response time
	
	if(triggerState == HIGH){
		vca1.noteOn();
	} else {
		vca1.noteOff();
	}
	
}