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
#include "synth_burst.h"

#define VERSION 1
#define HOLD_TRIGGER 0

// Audio Objects
AudioSynthWaveformModulated		osc1; // main oscillator
AudioEnvelopeAR					envelope1;
AudioEffectMultiply				vca1; // main oscilator envelope
AudioFilterStateVariable		vcf1; // main oscillator filter
AudioSynthWaveformDc			dc1;
AudioSynthWaveformModulated		lfo1;
AudioEnvelopeAR					eg1;
AudioEffectMultiply				vca2;
AudioSynthWaveformDc			dc2;
AudioSynthWaveformModulated		lfo2;
AudioEnvelopeAR					eg2;
AudioEffectMultiply				vca3;
AudioMixer4						modmix1;
AudioMixer4						modmix2;
AudioSynthBurst					impulse1;

AudioSynthNoiseWhite			noise1;
AudioEnvelopeAR					eg_noise1;
AudioEffectMultiply				vca_noise1; // main oscilator envelope
AudioFilterStateVariable		vcf_noise1; // main oscillator filter

AudioAnalyzeRMS					rms1; // RMS analyzer for LED indicator
AudioMixer4						mix1;
AudioFilterStateVariable		vcf_mix1; // main oscillator filter
AudioMixer4						mixMaster; // master mixer
AudioAmplifier					inverter; // invert waveform to have the correct phase (inverting opamp configuration)

AudioOutputAnalog				out1;

// Audio Object connections
AudioConnection					patchCordOsc1(osc1, 0, vca1, 0); 		// osc1 -> vca1
AudioConnection					patchCordVca1(envelope1, 0, vca1, 1); 	// env1 -> vca1
AudioConnection					patchCordVcf1(vca1, 0, vcf1, 0);		// vca1 -> vcf1

// Modulation Sources
AudioConnection					patchCordfm(dc1, 0, modmix1, 0);
AudioConnection					patchCordfm1(lfo1, 0, modmix1, 1);
AudioConnection					patchCordfm2(modmix1, 0, vca2, 0);
AudioConnection					patchCordfm3(eg1, 0, vca2, 1);
AudioConnection					patchCordffm(dc2, 0, modmix2, 0);
AudioConnection					patchCordffm1(lfo2, 0, modmix2, 1);
AudioConnection					patchCordffm2(modmix2, 0, vca3, 0);
AudioConnection					patchCordffm3(eg2, 0, vca3, 1);

AudioConnection					patchCordfm4(vca2, 0, osc1, 0);			// osc1 frequency modulation
AudioConnection					patchCordfm5(vca2, 0, vcf1, 1);			// osc1 filter frequency modulation
AudioConnection					patchCordfm7(vca2, 0, vcf_noise1, 1);	
AudioConnection					patchCordfm6(vca3, 0, vcf_mix1, 1);

AudioConnection					patchCordNoise1(noise1, 0, vca_noise1, 0); 		// osc1 -> vca1
AudioConnection					patchCordVca2(eg_noise1, 0, vca_noise1, 1); 	// env1 -> vca1
AudioConnection					patchCordVcf2(vca_noise1, 0, vcf_noise1, 0);		// vca1 -> vcf1

AudioConnection					patchCord4(vcf1, 0, mix1, 0);
AudioConnection					patchCord5(impulse1, 0, mix1, 3);
AudioConnection					patchCord3(vcf_noise1, 2, mix1, 2);  // noise filter high pass
AudioConnection					patchCord6(mix1, 0, vcf_mix1, 0);
AudioConnection					patchCordFilterMix1(vcf_mix1, 2, mixMaster, 0);
AudioConnection					patchCordFilterMix2(vcf_mix1, 1, mixMaster, 1);
AudioConnection					patchCordFilterMix3(vcf_mix1, 0, mixMaster, 2);
AudioConnection					patchCordoutlrms(mixMaster, 0, rms1, 0);
AudioConnection					patchCordinv(mixMaster, 0, inverter, 0);
AudioConnection					patchCordout(inverter, 0, out1, 0);

CallistoHAL callisto;

float frequency = 30;
float cutoff = 30;
float decay = 40;
float sourceMix = 0;
float depth = 0;
float rate = 0;
float click = 0;
float resonance = 0.7;
float frequencyHarmonics = 30;

uint32_t lastTrigger = 0;
uint8_t lastMode = 0;
uint8_t lastFilterMode = 0;

void setup(){
	callisto.setModeCallback(MODE_A, modeAChanged);
	callisto.setModeCallback(MODE_B, modeBChanged);

	callisto.setTriggerChangeCallback(triggerChange);
	
	AudioMemory(64);
	dc1.amplitude(0.5);
	dc2.amplitude(0.5);
	osc1.begin(WAVEFORM_TRIANGLE);
	osc1.frequency(40);
	osc1.amplitude(1.0);
	osc1.frequencyModulation(4.0);
	
	lfo1.begin(WAVEFORM_SINE);
	lfo1.frequency(30);
	lfo1.amplitude(0.25);
	lfo1.offset(0.25);
	lfo2.begin(WAVEFORM_SINE);
	lfo2.frequency(30);
	lfo2.amplitude(0.25);
	lfo2.offset(0.25);
	vcf1.frequency(60);
	vcf1.octaveControl(4.0);
	
	vcf_mix1.frequency(60);
	vcf_mix1.octaveControl(4.0);
	vcf_mix1.resonance(1.15);
	
	modmix1.gain(0,1.0);
	modmix1.gain(1,1.0);
	modmix2.gain(0,1.0);
	modmix2.gain(1,1.0);
	
	impulse1.frequency(10000);
	impulse1.amplitude(0.5);
	
	eg1.begin();
	eg2.begin();
	
	envelope1.begin();
	envelope1.releaseTime(100);
	
	eg_noise1.begin();
	eg_noise1.releaseTime(100);
	vcf_noise1.frequency(60);
	vcf_noise1.resonance(5.0);
	vcf_noise1.octaveControl(4.0);
	noise1.amplitude(1.0);
	
	mix1.gain(0, 1.0);
	mix1.gain(1, 0.0);
	mix1.gain(2, 0.0);
	mix1.gain(3, 1.0);
	
	mixMaster.gain(0, 1.0);
	mixMaster.gain(1, 0.0);
	mixMaster.gain(2, 0.0);
	inverter.gain(-0.9); // invert and reduce gain to avoid clipping on output opamp.
	
	callisto.setMode(MODE_A, 2);
	callisto.setMode(MODE_B, 2);
}

void loop(){
	
	if(millis() - lastTrigger >= 5){
		eg1.noteOff();
		eg2.noteOff();
		eg_noise1.noteOff();
		
		if(!HOLD_TRIGGER)
			envelope1.noteOff();
	}
	
	callisto.update();
	
	decay = max(callisto.readPotNorm(UI_F) * 1000.0, 10.0);
	cutoff = FREQ_MID_C * pow(2.0, callisto.readPotNorm(UI_D)*7.0-3.0);
	rate = callisto.readPotNorm(UI_E) * decay;
	depth = callisto.readPotNorm(UI_A);
	click = callisto.readPotNorm(UI_F);
	resonance = callisto.readPotNorm(UI_B) * 1.15;
	
	AudioNoInterrupts();
	osc1.frequency(callisto.readPitch());
	vcf1.frequency(callisto.readPitch() * 4.0);
	vcf_noise1.frequency(callisto.readPitch());
	vcf_mix1.frequency(cutoff);
	//vcf_mix1.resonance(resonance);
	envelope1.releaseTime(decay);
	
	lfo1.frequency(FREQ_MID_C * pow(2.0, callisto.readPotNorm(UI_E)*4.0-1.0));
	lfo2.frequency(FREQ_MID_C * pow(2.0, callisto.readPotNorm(UI_E)*2.0-4.0));
	
	modmix1.gain(0,max((1.0 - depth) * 2.0 - 1.0, 0.0));
	modmix1.gain(1,max(depth * 2.0 - 1.0, 0.0));
	
	modmix2.gain(0,max((1.0 - resonance) * 2.0 - 1.0, 0.0));
	modmix2.gain(1,max(resonance * 2.0 - 1.0, 0.0));
	
	//osc1.amplitude(1.0 - click);
	//noise1.amplitude(click);
	//impulse1.amplitude(click);
	
	eg1.releaseTime(rate);
	eg2.releaseTime(rate);
	eg_noise1.releaseTime(decay);
  
	AudioInterrupts();
	
	if(rms1.available()){
		float rms = rms1.read();
		callisto.setTrigLED(rms * 255);
	}
}

void modeAChanged(int mode){
	Serial.print("Mode changed: ");
	Serial.println(mode);
	
	AudioNoInterrupts();
	mix1.gain(lastMode, 0.0);
	mix1.gain(mode, 1.0);
	AudioInterrupts();
	
	lastMode = mode;
}

void modeBChanged(int mode){
	Serial.print("Filter changed: ");
	Serial.println(mode);
	
	AudioNoInterrupts();
	mixMaster.gain(lastFilterMode, 0.0);
	mixMaster.gain(mode, 1.0);
	AudioInterrupts();
	
	lastFilterMode = mode;
}

void triggerChange(){
	bool triggerState = !digitalReadFast(TRIGIN_PIN); // override any library calls for faster response TODO compare response time
	
	if(triggerState == HIGH){
		osc1.sync();
		lfo1.sync();
		lfo2.sync();
		envelope1.noteOn();
		eg1.noteOn();
		eg2.noteOn();
		impulse1.begin();
		eg_noise1.noteOn();
		lastTrigger = millis();
		
		//Serial.println(resonance);
	} else {
		eg1.noteOff();
		eg2.noteOff();
		envelope1.noteOff();
		eg_noise1.noteOff();
	}
	
}