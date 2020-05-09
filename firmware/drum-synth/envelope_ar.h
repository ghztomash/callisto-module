/* Callisto Eurorack Module - AR Exponential Envelope Module
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

#ifndef envelope_ar_h_
#define envelope_ar_h_

#include <Arduino.h>
#include "AudioStream.h"
#include "arm_math.h"


class AudioEnvelopeAR : public AudioStream
{
public:
	AudioEnvelopeAR(void) : AudioStream(0,NULL),
		running(0), a(0), xn(0), yn1(0) {
	}

	void attackTime(float millis) {
		millis /= 1000.0;
		float tau = millis * (1.0 - 2.0/3.0);
		attackT = tau / (tau + 1.0 / AUDIO_SAMPLE_RATE_EXACT) * UINT32_MAX;
	}
	
	void releaseTime(float millis) {
		millis /= 1000.0;
		float tau = millis * (1.0 - 2.0/3.0);
		releaseT = tau / (tau + 1.0 / AUDIO_SAMPLE_RATE_EXACT) * UINT32_MAX;
	}
	
	void noteOn() {
		xn = INT32_MAX;
		running = 1;
		a = attackT;
	}
	
	void noteOff() {
		xn = 0;
		running = 1;
		a = releaseT;
	}
	
	void begin() {
		a = 0;
		xn = 0;
		yn1 = 0;
		attackTime(0.0);
		releaseTime(0.0);
		running = 0;
	}
	
	bool isRunning(){
		return running;
	}

	virtual void update(void);

private:
	uint8_t	running;
	uint32_t attackT;
	uint32_t releaseT;
	uint32_t a;
	uint32_t xn;
	uint32_t yn1;
};

#endif
