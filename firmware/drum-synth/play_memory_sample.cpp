

#include "play_memory_sample.h"
#include "utility/dspinst.h"


void AudioPlayMemorySample::play(const unsigned int *data, unsigned int size)
{
	playing = 0;
	format = *data++;
	next = data;
	beginning = data;
	length = format & 0xFFFFFF;
	playing = format >> 24;
	sampleSize = (int)(length/2) << 15;
	phase = 0;
}

void AudioPlayMemorySample::play()
{
	playing = 0;
	next = beginning;
	length = format & 0xFFFFFF;
	playing = format >> 24;
	phase = 0;
}

void AudioPlayMemorySample::setSample(const unsigned int *data, unsigned int size)
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
	int16_t *out, *pmod;
	uint32_t tmp32;
	int i;
	uint32_t index;
	int32_t local_phase_increment;

	if (!playing) return;
	block = allocate();
	if (block == NULL) return;
	
	blockb = receiveReadOnly(0);

	//Serial.write('.');

	out = block->data;
	pmod = blockb->data;
	in = next;
	//s0 = prior;
	
	local_phase_increment = phase_incr + *pmod;
	if (local_phase_increment <= 0)
		local_phase_increment = 1;
	
	
	//Serial.print(phase_incr);
	//Serial.print('\t');
	//Serial.print(*pmod);
	//Serial.print('\t');
	//Serial.println(local_phase_increment);

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
				//Serial.println(*pmod);
				
				phase += local_phase_increment; // + mod * pmod;
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
	uint32_t b2m;

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