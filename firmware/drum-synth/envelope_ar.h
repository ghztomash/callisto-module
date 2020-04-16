

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
		//a = tau / (tau + 1.0 / AUDIO_SAMPLE_RATE_EXACT) * INT32_MAX;
		attackT = tau / (tau + 1.0 / AUDIO_SAMPLE_RATE_EXACT) * UINT32_MAX;
	}
	
	void releaseTime(float millis) {
		millis /= 1000.0;
		float tau = millis * (1.0 - 2.0/3.0);
		//a = tau / (tau + 1.0 / AUDIO_SAMPLE_RATE_EXACT) * INT32_MAX;
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
		running = 1;
	}

	virtual void update(void);

private:
	uint8_t	running;
	uint64_t attackT;
	uint64_t releaseT;
	uint64_t a;
	uint64_t xn;
	uint64_t	yn1;
};

#endif
