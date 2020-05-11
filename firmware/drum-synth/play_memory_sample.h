/* Callisto Eurorack Module - Sample Playback Module
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
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
 
#ifndef play_memory_s_h_
#define play_memory_s_h_

#include "AudioStream.h"
#include "Arduino.h"

class AudioPlayMemorySample : public AudioStream
{
public:
	AudioPlayMemorySample(void) : AudioStream(1, inputQueueArray), playing(0), phase_incr(1), modulation_factor(0) { }
	void play();
	void setSample(const unsigned int *data);
	void stop(void);
	bool isPlaying(void) { return playing; }
	uint32_t positionMillis(void);
	uint32_t lengthMillis(void);
	uint32_t lengthBytes(void);
	void setSpeed(float);
	float getSpeed();
	virtual void update(void);
	
	void frequencyModulation(float octaves) {
		if (octaves > 8.0) {
			octaves = 8.0;
		} else if (octaves < 0.0) {
			octaves = 0.0;
		}
		
		octaves = pow(2.0, octaves) - 1.0;
		
		modulation_factor = INT16_MAX * octaves;
	}
	
//private:
	const unsigned int *next;
	const unsigned int *beginning;
	float length;
	uint32_t format;
	volatile uint8_t playing;
	// volatile prevents the compiler optimizing out the frequency function
	volatile int32_t phase_incr;
	uint32_t sampleSize;
	int64_t modulation_factor;

	audio_block_t *inputQueueArray[1];
	
	uint32_t phase;
};

#endif
