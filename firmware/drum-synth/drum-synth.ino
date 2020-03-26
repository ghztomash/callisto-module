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
#include "envelope_ar.h"

// Audio Objects
AudioSynthWaveformModulated		osc1; // main oscillator
AudioEnvelopeAR					envelope1;
AudioEffectMultiply				vca1; // main oscilator envelope
AudioSynthWaveformDc			dc1;
AudioSynthWaveformModulated		lfo1;
AudioEnvelopeAR					eg1;
AudioEffectMultiply				vca2;
AudioMixer4						modmix1;
AudioAnalyzeRMS					rms1; // RMS analyzer for LED indicator
AudioMixer4						mix1;
AudioMixer4						mixMaster; // master mixer
AudioAmplifier					inverter; // invert waveform to have the correct phase (inverting opamp configuration)

AudioOutputAnalog				out1;

// Audio Object connections
AudioConnection					patchCordOsc1(osc1, 0, vca1, 0); // osc1 -> vca1
AudioConnection					patchCordVCA1(envelope1, 0, vca1, 1); // env1 -> vca1
AudioConnection					patchCordfm(dc1, 0, modmix1, 0);
AudioConnection					patchCordfm1(lfo1, 0, modmix1, 1);
AudioConnection					patchCordfm2(modmix1, 0, vca2, 0);
AudioConnection					patchCordfm4(eg1, 0, vca2, 1);
AudioConnection					patchCordfm3(vca2, 0, osc1, 0);
AudioConnection					patchCord4(vca1, 0, mix1, 0);
AudioConnection					patchCord6(mix1, 0, mixMaster, 0);
AudioConnection					patchCordoutlrms(mixMaster, 0, rms1, 0);
AudioConnection					patchCordinv(mixMaster, 0, inverter, 0);
AudioConnection					patchCordout(inverter, 0, out1, 0);

CallistoHAL callisto;

float frequency = 30;
float decay = 40;
float depth = 0;
float width = 0;
float frequencyHarmonics = 30;

uint32_t lastTrigger = 0;

void setup(){
	callisto.setModeCallback(MODE_A, modeAChanged);
	callisto.setModeCallback(MODE_B, modeBChanged);

	callisto.setTriggerChangeCallback(triggerChange);
	
	AudioMemory(64);
	dc1.amplitude(0.5);
	osc1.begin(WAVEFORM_SINE);
	osc1.frequency(40);
	osc1.amplitude(1.0);
	osc1.frequencyModulation(0);
	
	lfo1.begin(WAVEFORM_SINE);
	lfo1.frequency(30);
	lfo1.amplitude(0.9);
	
	modmix1.gain(0,1.0);
	modmix1.gain(1,0.0);
	
	//eg1.delay(0);
	//eg1.hold(0);
	//eg1.attack(0);
	//eg1.sustain(0);
	//eg1.decay(40);
	eg1.begin();
	
	envelope1.begin();
	envelope1.attack(100);
	
	mix1.gain(0, 1.0);
	mixMaster.gain(0, 1.0);
	inverter.gain(-0.9); // invert and reduce gain to avoid clipping on output opamp.
}

void loop(){
	
	if(millis() - lastTrigger > 5){
		eg1.noteOff();
	}
	
	callisto.update();
	
	decay = max(callisto.readPotNorm(UI_D) * 1000.0, 10.0);
	width = callisto.readPotNorm(UI_A) * decay;
	depth = callisto.readPotNorm(UI_F) * 4.0;
	
	AudioNoInterrupts();
	osc1.frequency(callisto.readPitch());
	//vca1.release(decay);
	//vca1.decay(decay);
	envelope1.attack(decay);
	osc1.frequencyModulation(depth);
	
	eg1.attack(width);
	//eg1.decay(width);
  
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
		osc1.sync();
		//vca1.noteOn();
		eg1.noteOn();
		envelope1.noteOn();
		lastTrigger = millis();
		//Serial.println(decay);
	} else {
		//vca1.noteOff();
		eg1.noteOff();
		envelope1.noteOff();
	}
	
}