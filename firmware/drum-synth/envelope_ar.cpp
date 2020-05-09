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

#include <Arduino.h>
#include "envelope_ar.h"
#include "arm_math.h"
#include "utility/dspinst.h"

void AudioEnvelopeAR::update(void)
{
	audio_block_t *block;
	int16_t *bp;
	uint32_t yn;
	uint32_t i;

	if (!running) {
		return;
	}
	
	block = allocate();
	if (!block) {
		return;
	}
	bp = block->data;
	
	for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
		yn = (yn1 * (uint64_t)a >> 32) + (xn * (uint64_t)(UINT32_MAX - a) >> 32);  
		yn1 = yn;
		*bp++ = yn >> 16;
		
		if(yn == xn){
			running = 0;
		}
	}

	transmit(block, 0);
	release(block);
}