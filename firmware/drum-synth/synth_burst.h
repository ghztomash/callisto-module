

#ifndef synth_burst_h_
#define synth_burst_h_

#include <Arduino.h>
#include "AudioStream.h"
#include "arm_math.h"

#define INT_TWOPI UINT32_MAX
#define INT_PI INT_TWOPI/2

#define M_PI 3.14159265358979323846
#define F_TWOPI M_PI*2

class AudioSynthBurst : public AudioStream
{
public:
	AudioSynthBurst(void) : AudioStream(0,NULL),
		phase_accumulator(0), phase_increment(0), incrementf(0), lastMagnitude(0),
		magnitude(0) {
	}

	void frequency(float freq) {
		if (freq < 0.0) {
			freq = 0.0;
		} else if (freq > AUDIO_SAMPLE_RATE_EXACT / 2) {
			freq = AUDIO_SAMPLE_RATE_EXACT / 2;
		}
		incrementf = freq * ((float32_t)INT_TWOPI / (float32_t)AUDIO_SAMPLE_RATE_EXACT);
	}
	void amplitude(float n) {	// 0 to 1.0
		if (n < 0) {
			n = 0;
		} else if (n > 1.0) {
			n = 1.0;
		}
		magnitude = n * 65536.0;
	}
	void begin() {
		cycles_count = 0;
		phase_accumulator = 0;
		lastMagnitude = 1;
		number_of_cycles = 2;
		running = 1;
	}
	void begin(float t_amp, float t_freq) {
		amplitude(t_amp);
		frequency(t_freq);
		cycles_count = 0;
		phase_accumulator = 0;
		lastMagnitude = 1;
		number_of_cycles = 2;
		running = 1;
	}
	void begin(float t_amp, float t_freq, uint8_t t_count) {
		amplitude(t_amp);
		frequency(t_freq);
		cycles_count = 0;
		number_of_cycles = t_count*2;
		phase_accumulator = 0;
		lastMagnitude = 1;
		running = 1;
		
		//Serial.println("Burst~");
	}
	virtual void update(void);

private:
	uint32_t phase_accumulator;
	uint32_t phase_increment;
	float32_t incrementf;
	uint32_t number_of_cycles;
	uint32_t cycles_count;
	uint8_t	 running;
	int16_t	 lastMagnitude;
	int32_t  magnitude;
};

#endif
