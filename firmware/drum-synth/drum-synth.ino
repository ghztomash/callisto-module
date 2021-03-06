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
#include "play_memory_sample.h"
#include "filter_variable.h"
#include "synth_waveform.h"

#include "AudioSampleCKick.h"
#include "AudioSampleCsnare.h"
#include "AudioSampleCclap.h"
#include "AudioSampleChat1.h"
#include "AudioSampleChat2.h"

#define HOLD_TRIGGER 0

// Audio Objects

AudioPlayMemorySample			sampleKick1;
AudioPlayMemorySample			sampleSnare1;
AudioPlayMemorySample			sampleSnare2;
AudioPlayMemorySample			sampleHat1;
AudioPlayMemorySample			sampleHat2;

AudioFilterStateVariableGhz		vcfKick1;

AudioSynthWaveformDc			dcMod1;
AudioSynthWaveformModulatedGhz	lfoMod1;
AudioEnvelopeAR					egMod1;
AudioMixer4						modmix1;

AudioSynthWaveformModulatedGhz	osc1;
AudioSynthWaveformModulatedGhz	osc2;
AudioSynthWaveformModulatedGhz	osc3;
AudioSynthWaveformModulatedGhz	osc4;

AudioSynthWaveformDc			dc2;

AudioSynthNoisePink				noise1;
AudioFilterStateVariableGhz		vcf_noise1; // filter

AudioMixer4						mixKick;
AudioMixer4						mixSnare;
AudioMixer4						mixHat;
AudioMixer4						mixMulti;
AudioMixer4						mixOSCNoise;
AudioMixer4						mixInstrument;

AudioMixer4						mixShaper;
AudioEffectWaveshaper			waveshaper;

AudioEnvelopeAR					eg1;
AudioEnvelopeAR					egSnare1;
AudioFilterStateVariableGhz		vcf1; // main filter
AudioFilterStateVariableGhz		vcf2; // main filter

AudioMixer4						mixMaster; // master mixer
AudioAmplifier					inverter; // invert waveform to have the correct phase (inverting opamp configuration)
AudioAnalyzeRMS					rms1; // RMS analyzer for LED indicator

AudioOutputAnalog				out1;

// Audio Object connections

AudioConnection					patchCordMulti1(osc1, 0, mixMulti, 0);
AudioConnection					patchCordMulti2(osc2, 0, mixMulti, 1);
AudioConnection					patchCordMulti3(osc3, 0, mixMulti, 2);
AudioConnection					patchCordMulti4(osc4, 0, mixMulti, 3);

AudioConnection					patchCordKickMix1(osc1, 0, mixKick, 0);
AudioConnection					patchCordKickMix2(mixMulti, 0, mixKick, 1);
AudioConnection					patchCordKickMix3(sampleKick1, 0, mixKick, 2);
AudioConnection					patchCordKickMix4(mixKick, 0, vcfKick1, 0);

AudioConnection					patchCordNoise1(noise1, 0, vcf_noise1, 0);

AudioConnection					patchCordonSnare1(osc1, 0, egSnare1, 0);
AudioConnection					patchCordonSnare3(egSnare1, 0, mixOSCNoise, 0);
AudioConnection					patchCordonSnare4(vcf_noise1, 1, mixOSCNoise, 1);

AudioConnection					patchCordSnareMix1(mixOSCNoise, 0, mixSnare, 0);
AudioConnection					patchCordSnareMix2(sampleSnare1, 0, mixSnare, 1);
AudioConnection					patchCordSnareMix3(sampleSnare2, 0, mixSnare, 2);

AudioConnection					patchCordHatMix1(vcf_noise1, 2, mixHat, 0);
AudioConnection					patchCordHatMix2(sampleHat1, 0, mixHat, 1);
AudioConnection					patchCordHatMix3(sampleHat2, 0, mixHat, 2);

AudioConnection					patchCordInstMix1(vcfKick1, 0, mixInstrument, 0);
AudioConnection					patchCordInstMix2(mixSnare, 0, mixInstrument, 1);
AudioConnection					patchCordInstMix3(mixHat, 0, mixInstrument, 2);

AudioConnection					patchCordVca3(mixInstrument, 0, mixShaper, 0); 		// osc1 -> vca1
AudioConnection					patchCordVca4(mixShaper, 0, eg1, 0); 		// osc1 -> vca1

AudioConnection					patchCordVcf1(eg1, 0, vcf1, 0);		// vca1 -> vcf1
AudioConnection					patchCordVcf2(vcf1, 0, vcf2, 0); // DJ style Filter

// Modulation Sources
AudioConnection					patchCordfm1(dcMod1, 0, modmix1, 0);
AudioConnection					patchCordfm2(lfoMod1, 0, modmix1, 1);
AudioConnection					patchCordfm3(modmix1, 0, egMod1, 0);

AudioConnection					patchCordfm5(egMod1, 0, osc1, 0);			// osc1 frequency modulation
AudioConnection					patchCordfm6(egMod1, 0, osc2, 0);
AudioConnection					patchCordfm7(egMod1, 0, osc3, 0);
AudioConnection					patchCordfm8(egMod1, 0, osc4, 0);

AudioConnection					patchCordfm9(egMod1, 0, vcf_noise1, 1);
AudioConnection					patchCordfm9x(egMod1, 0, vcfKick1, 1);
AudioConnection					patchCordfm10(egMod1, 0, sampleKick1, 0);
AudioConnection					patchCordfm11(egMod1, 0, sampleSnare1, 0);
AudioConnection					patchCordfm12(egMod1, 0, sampleSnare2, 0);
AudioConnection					patchCordfm13(egMod1, 0, sampleHat1, 0);
AudioConnection					patchCordfm14(egMod1, 0, sampleHat2, 0);

AudioConnection					patchCordFilt2(vcf2, 2, mixMaster, 1);

AudioConnection					patchCordoutlrms(mixMaster, 0, rms1, 0);
AudioConnection					patchCordinv(mixMaster, 0, inverter, 0);
AudioConnection					patchCordout(inverter, 0, out1, 0);

float WAVESHAPE[] = {
  -0.964028, -0.941376, -0.905148, -0.848284, -0.761594, -0.635149, -0.462117, -0.244919, 
  0.000000,   0.244919, 0.462117, 0.635149, 0.761594, 0.848284, 0.905148, 0.941376, 0.964028
};

CallistoHAL callisto;

float frequency = 30;
float sampleSpeed = 1;
float cutoffLow = 30;
float cutoffHigh = 30;
float decay = 40;
float sourceMix = 0;
float depth = 0;
float rate = 0;
float width = 0;
float click = 0;
float cutoff = 0.7;
float frequencyHarmonics = 30;
float overdrive = 0;

volatile uint8_t trigger = 0;
uint32_t lastTrigger = 0;
volatile uint8_t lastMode = 0;
volatile uint8_t lastFilterMode = 0;

void setup(){
	delay(100 + random(100)); // reduce power consumption spike
	
	callisto.setModeCallback(MODE_A, modeAChanged);
	callisto.setModeCallback(MODE_B, modeBChanged);

	callisto.setTriggerCallback(triggerChange);
	
	AudioMemory(64);
	dcMod1.amplitude(0.5);
	osc1.begin(WAVEFORM_TRIANGLE);
	osc1.frequency(40);
	osc1.amplitude(1.0);
	osc1.frequencyModulation(4.0);
	
	osc2.begin(0.6, 40, WAVEFORM_SINE);
	osc3.begin(0.3, 50, WAVEFORM_SINE);
	osc4.begin(0.2, 60, WAVEFORM_SINE);
	osc2.frequencyModulation(4.0);
	osc3.frequencyModulation(4.0);
	osc4.frequencyModulation(4.0);

	vcf1.frequency(30);
	vcf2.frequency(30);
	vcf1.resonance(0.65);
	vcf2.resonance(0.65);
	vcfKick1.frequency(30);
	vcfKick1.octaveControl(4.0);
	
	lfoMod1.begin(WAVEFORM_SINE);
	lfoMod1.frequency(30);
	lfoMod1.amplitude(0.75);
	
	modmix1.gain(0,1.0);
	modmix1.gain(1,0.0);
	
	eg1.begin();
	egSnare1.begin();
	egMod1.begin();
	
	vcf_noise1.frequency(60);
	vcf_noise1.resonance(4.0);
	vcf_noise1.octaveControl(4.0);
	noise1.amplitude(1.0);
	
	dc2.amplitude(1.0);
	
	sampleKick1.setSpeed(1.0);
	sampleSnare1.setSpeed(1.0);
	sampleSnare2.setSpeed(1.0);
	sampleHat1.setSpeed(1.0);
	sampleHat2.setSpeed(1.0);
	
	sampleKick1.frequencyModulation(3.0);
	sampleSnare1.frequencyModulation(3.0);
	sampleSnare2.frequencyModulation(3.0);
	sampleHat1.frequencyModulation(3.0);
	sampleHat2.frequencyModulation(3.0);
	
	sampleKick1.setSample(AudioSampleCkick);
	sampleSnare1.setSample(AudioSampleCsnare);
	sampleSnare2.setSample(AudioSampleCclap);
	sampleHat1.setSample(AudioSampleChat1);
	sampleHat2.setSample(AudioSampleChat2);
	
	mixMulti.gain(0, 1.0);
	mixMulti.gain(1, 0.8);
	mixMulti.gain(2, 0.8);
	mixMulti.gain(3, 0.8);
	
	mixOSCNoise.gain(0, 0.8);
	mixOSCNoise.gain(1, 1.0);
	mixOSCNoise.gain(2, 0.0);
	mixOSCNoise.gain(3, 0.0);
	
	mixKick.gain(0, 1.0);
	mixKick.gain(1, 0.0);
	mixKick.gain(2, 0.0);
	mixKick.gain(3, 0.0);
	
	mixSnare.gain(0, 1.0);
	mixSnare.gain(1, 0.0);
	mixSnare.gain(2, 0.0);
	mixSnare.gain(3, 0.0);
	
	mixHat.gain(0, 1.0);
	mixHat.gain(1, 0.0);
	mixHat.gain(2, 0.0);
	mixHat.gain(3, 0.0);
	
	mixInstrument.gain(0, 1.0); // kick
	mixInstrument.gain(1, 0.0); // snare
	mixInstrument.gain(2, 0.0); // hats
	
	mixMaster.gain(0, 0.0);
	mixMaster.gain(1, 1.0);
	mixMaster.gain(2, 0.0);
	mixMaster.gain(3, 0.0);
	
	waveshaper.shape(WAVESHAPE, 17);
	mixShaper.gain(0,1.0);
	
	inverter.gain(-0.9); // invert and reduce gain to avoid clipping on output opamp.
	
	callisto.setMode(MODE_A, 0);
	callisto.setMode(MODE_B, 0);
}

void loop(){
		
	callisto.update();
	sampleSpeed = constrain(pow(2.0, (callisto.readPotNorm(UI_C)-0.5)*4.0 + callisto.readCVVolt(UI_A)), 0.0, 8.0);
	decay = constrain((callisto.readPotNorm(UI_D) + callisto.readCVNorm(UI_D)) * 500.0, 10.0 , 500.0);
	cutoff = constrain(callisto.readPotNorm(UI_F) + callisto.readCVNorm(UI_F), 0.0, 1.0);
	cutoffLow = FREQ_MID_C * pow(2.0, min(cutoff * 2.0, 1.0 ) * 8.0-2.0);
	cutoffHigh = FREQ_MID_C * pow(2.0, max(cutoff * 2.0, 1.0 ) * 9.0-13.0);
	
	rate = constrain((callisto.readPotNorm(UI_E) + callisto.readCVNorm(UI_E)), 0.0, 1.0) * decay;
	depth = constrain(callisto.readPotNorm(UI_A) + callisto.readCVNorm(UI_C), 0.0, 1.0);
	width = constrain(callisto.readPotNorm(UI_E) + callisto.readCVNorm(UI_E), 0.0, 1.0) + 1.0;
	overdrive = constrain((callisto.readPotNorm(UI_B) + callisto.readCVNorm(UI_B)) * 2.0 + 1.0, 1.0, 3.0);
	
	AudioNoInterrupts();
		osc1.frequency(callisto.readPitch());
		
		vcf1.frequency(min(cutoffLow, 14000.0));
		vcf2.frequency(min(cutoffHigh, 14000.0));
		
		vcfKick1.frequency(min(callisto.readPitch() * 4.00, 14000.0));
		vcf_noise1.frequency(min(callisto.readPitch() * 4.00, 14000.0));
		
		osc2.frequency(callisto.readPitch() * 1.49829 * width);
		osc3.frequency(callisto.readPitch() * 2.0 * width * width);
		osc4.frequency(callisto.readPitch() * 2.99661 * width * width * width);
		
		lfoMod1.frequency(FREQ_MID_C * pow(2.0, (1.0 - callisto.readPotNorm(UI_E))*2.0-4.0));
		
		mixShaper.gain(0, overdrive);
		
		modmix1.gain(0,max((1.0 - depth) * 2.0 - 1.0, 0.0));
		modmix1.gain(1,max(depth * 2.0 - 1.0, 0.0));
		
		eg1.releaseTime(decay);
		egSnare1.releaseTime(decay/4.0);
		egMod1.releaseTime(rate);
		
		
		sampleKick1.setSpeed(sampleSpeed);
		sampleSnare1.setSpeed(sampleSpeed);
		sampleSnare2.setSpeed(sampleSpeed);
		sampleHat1.setSpeed(sampleSpeed);
		sampleHat2.setSpeed(sampleSpeed);
  
	AudioInterrupts();
	
	if (trigger){
		noInterrupts();
		trigger = 0;
		interrupts();
		
		AudioNoInterrupts();
		osc1.sync();
		osc2.sync();
		osc3.sync();
		osc4.sync();
		lfoMod1.sync();
		
		eg1.noteOn();
		egSnare1.noteOn();
		egMod1.noteOn();
		
		if ((lastMode == 0) & (lastFilterMode == 2))
			sampleKick1.play();
		if ((lastMode == 1) & (lastFilterMode == 1))
			sampleSnare1.play();
		if ((lastMode == 1) & (lastFilterMode == 2))
			sampleSnare2.play();
		if ((lastMode == 2) & (lastFilterMode == 1))
			sampleHat1.play();
		if ((lastMode == 2) & (lastFilterMode == 2))
			sampleHat2.play();
		AudioInterrupts();
		lastTrigger = millis();
	}
	
	if(millis() - lastTrigger >= 5){
		egMod1.noteOff();
		
		if(!HOLD_TRIGGER){
			eg1.noteOff();
			egSnare1.noteOff();
		}
			
	}
	
	if(rms1.available()){
		float rms = rms1.read();
		callisto.setTrigLED(rms * 255);
	}
}

void modeAChanged(int mode){
	
	AudioNoInterrupts();
		mixInstrument.gain(lastMode, 0.0);
		mixInstrument.gain(mode, 1.0);
	AudioInterrupts();
	
	lastMode = mode;
}

void modeBChanged(int mode){

	AudioNoInterrupts();
		mixKick.gain(lastFilterMode, 0.0);
		mixKick.gain(mode, 1.0);
		mixSnare.gain(lastFilterMode, 0.0);
		if(mode==0)
			mixSnare.gain(mode, 1.0);
		else
			mixSnare.gain(mode, 0.8);
		mixHat.gain(lastFilterMode, 0.0);
		if(mode==0)
			mixHat.gain(mode, 1.0);
		else
			mixHat.gain(mode, 0.8);
	AudioInterrupts();
	
	lastFilterMode = mode;
}

void triggerChange(){
	noInterrupts();
	trigger = 1;
	interrupts();
}