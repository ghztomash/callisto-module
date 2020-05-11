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

#include "play_memory_sample.h"
#include "utility/dspinst.h"

void AudioPlayMemorySample::play()
{
	playing = 0;
	next = beginning;
	length = format & 0xFFFFFF;
	playing = format >> 24;
	phase = 0;
}

void AudioPlayMemorySample::setSample(const unsigned int *data)
{
	playing = 0;
	format = *data++;
	next = data;
	beginning = data;
	length = format & 0xFFFFFF;
	sampleSize = (int)(length/2) << 15;
}

void AudioPlayMemorySample::stop(void)
{
	playing = 0;
}

void AudioPlayMemorySample::update(void)
{
	audio_block_t *block, *blockb;
	const unsigned int *in;
	int16_t *out;
	int64_t pmod = 0;
	uint32_t tmp32;
	int i;
	uint32_t index;
	int64_t local_phase_increment;

	if (!playing) return;
	
	block = allocate();
	if (!block) return;
	
	blockb = receiveReadOnly(0);

	out = block->data;
	
	if (blockb){
		pmod = *blockb->data * modulation_factor >> 15;
	}
	
	in = next;
	
	local_phase_increment = phase_incr + pmod;
	if (local_phase_increment <= 0)
		local_phase_increment = 1;

	switch (playing) {
	  case 0x81: // 16 bit PCM, 44100 Hz
		// this part mostly for my stuff
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i += 2) {
			index = phase >> 15;
			
			if (phase + local_phase_increment> sampleSize) {
					playing = 0;
					*out++ = 0;
					*out++ = 0;
					//break;
			} else {
					tmp32 = in[index];
					*out++ = (int16_t)(tmp32 & 65535);
					*out++ = (int16_t)(tmp32 >> 16);
			}	
			phase += local_phase_increment;
		}
		break;

	  default:
		release(block);
		playing = 0;
		return;
	}
	
	next = in;
	
	transmit(block);
	release(block);
	release(blockb);
}


#define B2M_88200 (uint32_t)((double)4294967296000.0 / AUDIO_SAMPLE_RATE_EXACT / 2.0)
#define B2M_44100 (uint32_t)((double)4294967296000.0 / AUDIO_SAMPLE_RATE_EXACT) // 97352592
#define B2M_22050 (uint32_t)((double)4294967296000.0 / AUDIO_SAMPLE_RATE_EXACT * 2.0)
#define B2M_11025 (uint32_t)((double)4294967296000.0 / AUDIO_SAMPLE_RATE_EXACT * 4.0)


uint32_t AudioPlayMemorySample::positionMillis(void)
{
	uint8_t p;
	const uint8_t *n, *b;
	uint32_t b2m;

	__disable_irq();
	p = playing;
	n = (const uint8_t *)next;
	b = (const uint8_t *)beginning;
	__enable_irq();
	switch (p) {
	  case 0x81: // 16 bit PCM, 44100 Hz
		b2m = B2M_88200;  break;
	  case 0x01: // u-law encoded, 44100 Hz
	  case 0x82: // 16 bits PCM, 22050 Hz
		b2m = B2M_44100;  break;
	  case 0x02: // u-law encoded, 22050 Hz
	  case 0x83: // 16 bit PCM, 11025 Hz
		b2m = B2M_22050;  break;
	  case 0x03: // u-law encoded, 11025 Hz
		b2m = B2M_11025;  break;
	  default:
		return 0;
	}
	if (p == 0) return 0;
	return ((uint64_t)(n - b) * b2m) >> 32;
}

uint32_t AudioPlayMemorySample::lengthMillis(void)
{
	uint8_t p;
	const uint32_t *b;
	uint32_t b2m;

	__disable_irq();
	p = playing;
	b = (const uint32_t *)beginning;
	__enable_irq();
	switch (p) {
	  case 0x81: // 16 bit PCM, 44100 Hz
	  case 0x01: // u-law encoded, 44100 Hz
		b2m = B2M_44100;  break;
	  case 0x82: // 16 bits PCM, 22050 Hz
	  case 0x02: // u-law encoded, 22050 Hz
		b2m = B2M_22050;  break;
	  case 0x83: // 16 bit PCM, 11025 Hz
	  case 0x03: // u-law encoded, 11025 Hz
		b2m = B2M_11025;  break;
	  default:
		return 0;
	}
	return ((uint64_t)(*(b - 1) & 0xFFFFFF) * b2m) >> 32;
}

uint32_t AudioPlayMemorySample::lengthBytes(void)
{
	const uint32_t *b;

	__disable_irq();
	b = (const uint32_t *)beginning;
	__enable_irq();
	return ((uint64_t)(*(b-1) & 0xFFFFFF) *4294967296000) >>32;
}

void AudioPlayMemorySample::setSpeed(float s)
{
	if( s < 0.01 )
		s = 0.01;
	//phase = sampleSize; // reverse?
		
	phase_incr = INT16_MAX * s;
}

float AudioPlayMemorySample::getSpeed(void)
{
	return phase_incr / (float) INT16_MAX;
}