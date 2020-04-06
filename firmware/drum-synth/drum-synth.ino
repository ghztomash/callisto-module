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
AudioEffectMultiply				modvca1;
AudioSynthWaveformDc			dc2;
AudioSynthWaveformModulated		lfo2;
AudioEnvelopeAR					eg2;
AudioEffectMultiply				modvca2;
AudioMixer4						modmix1;
AudioMixer4						modmix2;
AudioSynthBurst					impulse1;

AudioSynthWaveformModulated		osc2;
AudioEnvelopeAR					envelope2;
AudioEffectMultiply				vca2;
AudioFilterStateVariable		vcf2;
AudioSynthWaveformModulated		osc3;
AudioEnvelopeAR					envelope3;
AudioEffectMultiply				vca3;
AudioFilterStateVariable		vcf3;
AudioSynthWaveformModulated		osc4;
AudioEnvelopeAR					envelope4;
AudioEffectMultiply				vca4;
AudioFilterStateVariable		vcf4;
AudioSynthWaveformModulated		osc5;
AudioEnvelopeAR					envelope5;
AudioEffectMultiply				vca5;
AudioFilterStateVariable		vcf5;

AudioSynthNoiseWhite			noise1;
AudioEnvelopeAR					eg_noise1;
AudioEffectMultiply				vca_noise1; // main oscilator envelope
AudioFilterStateVariable		vcf_noise1; // main oscillator filter

AudioAnalyzeRMS					rms1; // RMS analyzer for LED indicator
AudioMixer4						mix1;
AudioMixer4						mix2;
AudioMixer4						mix3;
AudioMixer4						mixMaster; // master mixer
AudioAmplifier					inverter; // invert waveform to have the correct phase (inverting opamp configuration)

AudioOutputAnalog				out1;

// Audio Object connections
AudioConnection					patchCordOsc1(osc1, 0, vca1, 0); 		// osc1 -> vca1
AudioConnection					patchCordVca1(envelope1, 0, vca1, 1); 	// env1 -> vca1
AudioConnection					patchCordVcf1(vca1, 0, vcf1, 0);		// vca1 -> vcf1

AudioConnection					patchCordOsc2(osc2, 0, vca2, 0); 		// osc1 -> vca1
AudioConnection					patchCordVca2(envelope2, 0, vca2, 1); 	// env1 -> vca1
AudioConnection					patchCordVcf2(vca2, 0, vcf2, 0);		// vca1 -> vcf1
AudioConnection					patchCordOsc3(osc3, 0, vca3, 0); 		// osc1 -> vca1
AudioConnection					patchCordVca3(envelope3, 0, vca3, 1); 	// env1 -> vca1
AudioConnection					patchCordVcf3(vca3, 0, vcf3, 0);		// vca1 -> vcf1
AudioConnection					patchCordOsc4(osc4, 0, vca4, 0); 		// osc1 -> vca1
AudioConnection					patchCordVca4(envelope4, 0, vca4, 1); 	// env1 -> vca1
AudioConnection					patchCordVcf4(vca4, 0, vcf4, 0);		// vca1 -> vcf1
AudioConnection					patchCordOsc5(osc5, 0, vca5, 0); 		// osc1 -> vca1
AudioConnection					patchCordVca5(envelope5, 0, vca5, 1); 	// env1 -> vca1
AudioConnection					patchCordVcf5(vca5, 0, vcf5, 0);		// vca1 -> vcf1

AudioConnection					patchCordMix2(vcf2, 0, mix2, 0);
AudioConnection					patchCordMix3(vcf3, 0, mix2, 1);
AudioConnection					patchCordMix4(vcf4, 0, mix2, 2);
AudioConnection					patchCordMix5(vcf5, 0, mix2, 3);

// Modulation Sources
AudioConnection					patchCordfm(dc1, 0, modmix1, 0);
AudioConnection					patchCordfm1(lfo1, 0, modmix1, 1);
AudioConnection					patchCordfm2(modmix1, 0, modvca1, 0);
AudioConnection					patchCordfm3(eg1, 0, modvca1, 1);
AudioConnection					patchCordffm(dc2, 0, modmix2, 0);
AudioConnection					patchCordffm1(lfo2, 0, modmix2, 1);
AudioConnection					patchCordffm2(modmix2, 0, modvca2, 0);
AudioConnection					patchCordffm3(eg2, 0, modvca2, 1);

AudioConnection					patchCordfm4(modvca1, 0, osc1, 0);			// osc1 frequency modulation
AudioConnection					patchCordfm5(modvca1, 0, vcf1, 1);			// osc1 filter frequency modulation
AudioConnection					patchCordfm6(modvca1, 0, vcf_noise1, 1);	
//AudioConnection					patchCordfm8(modvca1, 0, osc2, 0);
//AudioConnection					patchCordfm9(modvca1, 0, osc3, 0);
//AudioConnection					patchCordfm10(modvca1, 0, osc4, 0);
//AudioConnection					patchCordfm11(modvca1, 0, osc5, 0);

AudioConnection					patchCordNoise1(noise1, 0, vca_noise1, 0); 		// osc1 -> vca1
AudioConnection					patchCordNoise2(eg_noise1, 0, vca_noise1, 1); 	// env1 -> vca1
AudioConnection					patchCordNoise3(vca_noise1, 0, vcf_noise1, 0);		// vca1 -> vcf1

AudioConnection					patchCord3(vcf1, 0, mix1, 0);
AudioConnection					patchCord4(mix2, 0, mix1, 1);
AudioConnection					patchCord5(vcf_noise1, 2, mix3, 1);  // noise filter high pass
AudioConnection					patchCord6(impulse1, 0, mix3, 0);
AudioConnection					patchCord7(mix1, 0, mixMaster, 0);
AudioConnection					patchCord8(mix3, 0, mixMaster, 1);
AudioConnection					patchCordoutlrms(mixMaster, 0, rms1, 0);
AudioConnection					patchCordinv(mixMaster, 0, inverter, 0);
AudioConnection					patchCordout(inverter, 0, out1, 0);

CallistoHAL callisto;

float frequency = 30;
float cutoff = 30;
float decayBody = 40;
float decayTransient = 40;
float sourceMix = 0;
float depth = 0;
float rate = 0;
float width = 0;
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
	
	osc2.begin(0.6, 40, WAVEFORM_SINE);
	osc3.begin(0.3, 50, WAVEFORM_SINE);
	osc4.begin(0.2, 60, WAVEFORM_SINE);
	osc5.begin(0.1, 70, WAVEFORM_SINE);
	osc2.frequencyModulation(4.0);
	osc3.frequencyModulation(4.0);
	osc4.frequencyModulation(4.0);
	osc5.frequencyModulation(4.0);
	vcf2.frequency(80);
	vcf3.frequency(100);
	vcf4.frequency(120);
	vcf5.frequency(140);
	vcf2.octaveControl(4.0);
	vcf3.octaveControl(4.0);
	vcf4.octaveControl(4.0);
	vcf5.octaveControl(4.0);
	
	lfo1.begin(WAVEFORM_SINE);
	lfo1.frequency(30);
	lfo1.amplitude(1.0);
	//lfo1.amplitude(0.25);
	//lfo1.offset(0.25);
	lfo2.begin(WAVEFORM_SINE);
	lfo2.frequency(30);
	lfo2.amplitude(0.25);
	lfo2.offset(0.25);
	vcf1.frequency(60);
	vcf1.octaveControl(4.0);
	
	modmix1.gain(0,1.0);
	modmix1.gain(1,0.0);
	modmix2.gain(0,0.0);
	modmix2.gain(1,0.0);
	
	impulse1.frequency(10000);
	impulse1.amplitude(1.0);
	
	eg1.begin();
	eg2.begin();
	
	envelope1.begin();
	envelope1.releaseTime(100);
	
	envelope2.begin();
	envelope3.begin();
	envelope4.begin();
	envelope5.begin();
	
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
	
	mix2.gain(0, 1.0);
	mix2.gain(1, 1.0);
	mix2.gain(2, 1.0);
	mix2.gain(3, 1.0);
	
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
		
		if(!HOLD_TRIGGER){
			envelope1.noteOff();
			envelope2.noteOff();
			envelope3.noteOff();
			envelope4.noteOff();
			envelope5.noteOff();
		}
			
	}
	
	callisto.update();
	sourceMix = callisto.readPotNorm(UI_F);
	decayBody = max(callisto.readPotNorm(UI_A) * 500.0, 10.0);
	decayTransient = max(callisto.readPotNorm(UI_B) * 500.0, 1.0);
	cutoff = FREQ_MID_C * pow(2.0, callisto.readPotNorm(UI_D)*7.0-3.0);
	
	//rate = callisto.readPotNorm(UI_E) * decayBody;

	depth = callisto.readPotNorm(UI_E);
	//resonance = callisto.readPotNorm(UI_B) * 1.15;
	width = callisto.readPotNorm(UI_E) + 1.0;
	
	AudioNoInterrupts();
	osc1.frequency(callisto.readPitch());
	vcf1.frequency(min(callisto.readPitch() * 4.00, 14000.0));
	
	vcf_noise1.frequency(cutoff);
	impulse1.frequency(cutoff);
	
	envelope1.releaseTime(decayBody);
	
	osc2.frequency(callisto.readPitch());
	osc3.frequency(callisto.readPitch() * 1.49829 * width);
	osc4.frequency(callisto.readPitch() * 2.0 * width * width);
	osc5.frequency(callisto.readPitch() * 2.99661 * width * width * width);
	vcf2.frequency(min(callisto.readPitch() * 4.0 , 14000.0));
	vcf3.frequency(min(callisto.readPitch() * 1.49829 * width * 4.0 , 14000.0));
	vcf4.frequency(min(callisto.readPitch() * 2.0 * width * width * 4.0 , 14000.0));
	vcf5.frequency(min(callisto.readPitch() * 2.99661 * width * width * width * 4.0 , 14000.0));
	//Serial.println(callisto.readPitch() * 4.0);
	
	envelope2.releaseTime(decayBody);
	envelope3.releaseTime(decayBody * 0.75);
	envelope4.releaseTime(decayBody * 0.5);
	envelope5.releaseTime(decayBody * 0.25);
	
	//lfo1.frequency(FREQ_MID_C * pow(2.0, callisto.readPotNorm(UI_E)*3.0-2.0));
	//lfo2.frequency(FREQ_MID_C * pow(2.0, callisto.readPotNorm(UI_E)*2.0-4.0));
	
	modmix1.gain(0, depth);
	//modmix1.gain(1,max(depth * 2.0 - 1.0, 0.0));
	
	//modmix2.gain(0,max((1.0 - resonance) * 2.0 - 1.0, 0.0));
	//modmix2.gain(1,max(resonance * 2.0 - 1.0, 0.0));
	
	mixMaster.gain(0, 1.0 - sourceMix);
	mixMaster.gain(1, sourceMix);
	
	eg1.releaseTime(decayBody / 4.0);
	eg2.releaseTime(rate);
	eg_noise1.releaseTime(decayTransient);
  
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
	mix3.gain(lastFilterMode, 0.0);
	mix3.gain(mode, 1.0);
	AudioInterrupts();
	
	lastFilterMode = mode;
}

void triggerChange(){
	bool triggerState = !digitalReadFast(TRIGIN_PIN); // override any library calls for faster response TODO compare response time
	
	if(triggerState == HIGH){
		osc1.sync();
		osc2.sync();
		osc3.sync();
		osc4.sync();
		osc5.sync();
		lfo1.sync();
		lfo2.sync();
		envelope1.noteOn();
		envelope2.noteOn();
		envelope3.noteOn();
		envelope4.noteOn();
		envelope5.noteOn();
		eg1.noteOn();
		eg2.noteOn();
		impulse1.begin();
		eg_noise1.noteOn();
		lastTrigger = millis();
		
		//Serial.println(callisto.readPotNorm(UI_E));
	} else {
		eg1.noteOff();
		eg2.noteOff();
		envelope1.noteOff();
		envelope2.noteOff();
		envelope3.noteOff();
		envelope4.noteOff();
		envelope5.noteOff();
		eg_noise1.noteOff();
	}
	
}