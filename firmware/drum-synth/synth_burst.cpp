

#include <Arduino.h>
#include "synth_burst.h"
#include "arm_math.h"
#include "utility/dspinst.h"


// uncomment for more accurate but more computationally expensive frequency modulation
//#define IMPROVE_EXPONENTIAL_ACCURACY


void AudioSynthBurst::update(void)
{
	audio_block_t *block;
	int16_t *bp;
	int16_t magnitude15;
	int16_t lg = lastMagnitude;
	uint32_t i, ph;
	const float32_t incf = incrementf;

	ph = phase_accumulator;
	if (magnitude == 0) {
		return;
	}
	if (!running) {
		return;
	}
	block = allocate();
	if (!block) {
		return;
	}
	bp = block->data;

	magnitude15 = signed_saturate_rshift(magnitude, 16, 1);
	for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
		if (ph <= INT_PI) {
			if(lg == 0){
				lg = 1;
				cycles_count++;
				if(cycles_count >= number_of_cycles){
					running = 0;
				}
			}
			if(!running)
				*bp++ = 0;
			else 
				*bp++ = magnitude15;
		} else {
			if(lg == 1){
				lg = 0;
				cycles_count++;
				if(cycles_count >= number_of_cycles){
					running = 0;
				}
			}
			if(!running)
				*bp++ = 0;
			else 
				*bp++ = -magnitude15;
		}
		ph += (uint32_t)incf;
	}
	phase_accumulator = ph;
	lastMagnitude = lg;

	transmit(block, 0);
	release(block);
}