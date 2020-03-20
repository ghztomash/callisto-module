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

// TODO Add HAL function prototypes here for reference.