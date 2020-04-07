

#include "play_memory_sample.h"
#include "utility/dspinst.h"


void AudioPlayMemorySample::play(const unsigned int *data, unsigned int size)
{

	playing = 0;
	prior = 0;
	format = *data++;
	next = data;
	beginning = data;
	length = format & 0xFFFFFF;
	env_lin_current = 0x7fff0000;
	playing = format >> 24;
	sampleSize = length/2;
	
	phase = 0;
}

void AudioPlayMemorySample::play()
{
	playing = 0;
	prior = 0;
	next = beginning;
	length = format & 0xFFFFFF;
	env_lin_current = 0x7fff0000;
	playing = format >> 24;
	
	if(phase_incr<0)
		phase=sampleSize;
	else
		phase = 0;
}

void AudioPlayMemorySample::setSample(const unsigned int *data, unsigned int size)
{
	playing = 0;
	prior = 0;
	format = *data++;
	next = data;
	beginning = data;
	length = format & 0xFFFFFF;
	sampleSize = length/2;
	//parseSample();
}

void AudioPlayMemorySample::stop(void)
{
	playing = 0;
}

extern "C" {
extern const int16_t ulaw_decode_table[256];
};

void AudioPlayMemorySample::update(void)
{
	audio_block_t *block;
	const unsigned int *in;
	int16_t *out;
	uint32_t tmp32, consumed;
	int16_t s0, s1, s2, s3, s4;
	int i;
	float mod;
	
	int32_t env_sqr_current; // the square of the linear value - inexpensive quasi exponential decay.
	uint32_t index;

	if (!playing) return;
	block = allocate();
	if (block == NULL) return;

	//Serial.write('.');

	out = block->data;
	in = next;
	s0 = prior;

	switch (playing) {
	  case 0x01: // u-law encoded, 44100 Hz
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i += 4) {//4
			index = phase;
			//tmp32 = *in++;
			tmp32 = in[index];
			*out++ = ulaw_decode_table[(tmp32 >> 0) & 255];
			*out++ = ulaw_decode_table[(tmp32 >> 8) & 255];
			*out++ = ulaw_decode_table[(tmp32 >> 16) & 255];
			*out++ = ulaw_decode_table[(tmp32 >> 24) & 255];
			
			phase += phase_incr;
			if((phase>sampleSize)||(phase<0)){
				playing = 0;
				break;
				}
		}
		consumed = 128*phase_incr;
		//consumed = 128;
		break;

	  case 0x81: // 16 bit PCM, 44100 Hz
		// this part mostly for my stuff
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i += 2) {
			index = phase;
			
			if(env_lin_current < 0x0000ffff)
				{
				  // If envelope has expired, then stuff zeros into output buffer.
				  *out++ = 0;
				  *out++ = 0;
				  playing = 0;
				}
			else{
				env_lin_current -= env_decrement;
				env_sqr_current = multiply_16tx16t(env_lin_current, env_lin_current) >> 14;
				//env_sqr_current = 0xffff;
				mod = env_sqr_current/65532.0;
				
				if(interpolation){
					tmp32 = in[index];
					uint32_t nextSample = in[index+1];
					int16_t diffA =((int16_t)(tmp32 >> 16)-(int16_t)(tmp32 & 65535))*((phase_incr/2.0)/(phase_incr));
					int16_t diffB =((int16_t)(nextSample & 65535)-(int16_t)(tmp32 >> 16))*((phase_incr/2.0)/(phase_incr));
					
					*out++ = (int16_t)(tmp32 & 65535)+diffA;
					*out++ = (int16_t)(tmp32 >> 16)+diffB;
					
				}else{
				//tmp32 = *in++;
					tmp32 = in[index];
					//*out++ = (int16_t)(tmp32 & 65535);
					//*out++ = (int16_t)(tmp32 >> 16);
					
					int32_t val1 = signed_multiply_32x16b(env_sqr_current, tmp32);
					int32_t val2 = signed_multiply_32x16t(env_sqr_current, tmp32);
					val1 = signed_saturate_rshift(val1, 16, 0);
					val2 = signed_saturate_rshift(val2, 16, 0);
					//uint32_t ppp = pack_16b_16b(val2, val1);
					
					*out++ = (int16_t)val1;
					*out++ = (int16_t)val2;
				}
				
				phase += phase_incr + mod * pmod;
				if((phase>sampleSize)||(phase<0)){
					playing = 0;
					break;
				}
			}
		}
		consumed = 128*phase_incr + mod * pmod;
		break;

	  case 0x02: // u-law encoded, 22050 Hz 
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i += 8) {
			tmp32 = *in++;
			s1 = ulaw_decode_table[(tmp32 >> 0) & 255];
			s2 = ulaw_decode_table[(tmp32 >> 8) & 255];
			s3 = ulaw_decode_table[(tmp32 >> 16) & 255];
			s4 = ulaw_decode_table[(tmp32 >> 24) & 255];
			*out++ = (s0 + s1) >> 1;
			*out++ = s1;
			*out++ = (s1 + s2) >> 1;
			*out++ = s2;
			*out++ = (s2 + s3) >> 1;
			*out++ = s3;
			*out++ = (s3 + s4) >> 1;
			*out++ = s4;
			s0 = s4;
		}
		consumed = 64;
		break;

	  case 0x82: // 16 bits PCM, 22050 Hz
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i += 4) {
			tmp32 = *in++;
			s1 = (int16_t)(tmp32 & 65535);
			s2 = (int16_t)(tmp32 >> 16);
			*out++ = (s0 + s1) >> 1;
			*out++ = s1;
			*out++ = (s1 + s2) >> 1;
			*out++ = s2;
			s0 = s2;
		}
		consumed = 64;
		break;

	  case 0x03: // u-law encoded, 11025 Hz
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i += 16) {
			tmp32 = *in++;
			s1 = ulaw_decode_table[(tmp32 >> 0) & 255];
			s2 = ulaw_decode_table[(tmp32 >> 8) & 255];
			s3 = ulaw_decode_table[(tmp32 >> 16) & 255];
			s4 = ulaw_decode_table[(tmp32 >> 24) & 255];
			*out++ = (s0 * 3 + s1) >> 2;
			*out++ = (s0 + s1)     >> 1;
			*out++ = (s0 + s1 * 3) >> 2;
			*out++ = s1;
			*out++ = (s1 * 3 + s2) >> 2;
			*out++ = (s1 + s2)     >> 1;
			*out++ = (s1 + s2 * 3) >> 2;
			*out++ = s2;
			*out++ = (s2 * 3 + s3) >> 2;
			*out++ = (s2 + s3)     >> 1;
			*out++ = (s2 + s3 * 3) >> 2;
			*out++ = s3;
			*out++ = (s3 * 3 + s4) >> 2;
			*out++ = (s3 + s4)     >> 1;
			*out++ = (s3 + s4 * 3) >> 2;
			*out++ = s4;
			s0 = s4;
		}
		consumed = 32;
		break;

	  case 0x83: // 16 bit PCM, 11025 Hz
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i += 8) {
			tmp32 = *in++;
			s1 = (int16_t)(tmp32 & 65535);
			s2 = (int16_t)(tmp32 >> 16);
			*out++ = (s0 * 3 + s1) >> 2;
			*out++ = (s0 + s1)     >> 1;
			*out++ = (s0 + s1 * 3) >> 2;
			*out++ = s1;
			*out++ = (s1 * 3 + s2) >> 2;
			*out++ = (s1 + s2)     >> 1;
			*out++ = (s1 + s2 * 3) >> 2;
			*out++ = s2;
			s0 = s2;
		}
		consumed = 32;
		break;

	  default:
		release(block);
		playing = 0;
		return;
	}
	
	prior = s0;
	next = in;
	
	//if (length*phase_incr > consumed) {
	//	length -= consumed;
	//} else {
	//	playing = 0;
	//}
	
	transmit(block);
	release(block);
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
	if(s<0)
		phase=sampleSize;
		
	phase_incr = s;
}

long double AudioPlayMemorySample::getPhase(void)
{
	return phase;
}

float AudioPlayMemorySample::getSpeed(void)
{
	return phase_incr;
}

void AudioPlayMemorySample::pitchMod(float p)
{
	if(p<0.0)
		pmod=0.0;
	else if(p>1)
		pmod=1;
	else
		pmod=p;
	pmod-=0.5;
}


void AudioPlayMemorySample::parseSample(){
	uint32_t tmp32;
	//buffer.clear();
	//buffer.reserve(44161);
	
	//for(int i=0; i<=sampleSize; i++){
	//	tmp32 = beginning[i];
	//	buffer.push_back((int16_t)(tmp32 & 65535));
	//	buffer.push_back((int16_t)(tmp32 >> 16));
	//}
}