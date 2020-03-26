

#include <Arduino.h>
#include "envelope_ar.h"
#include "arm_math.h"
#include "utility/dspinst.h"

void AudioEnvelopeAR::update(void)
{
	audio_block_t *block;
	int16_t *bp;
	int64_t yn;
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
		yn = (yn1 * a >> 31) + (xn * ( INT32_MAX - a) >> 31);  
		yn1 = yn;
		*bp++ = yn >> 16;
	}

	transmit(block, 0);
	release(block);
}