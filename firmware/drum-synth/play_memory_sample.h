
#ifndef play_memory_s_h_
#define play_memory_s_h_

#include "AudioStream.h"
#include "Arduino.h"

// fractional bits for sample index precision
#define SAMPLE_F_BITS 16
#define SAMPLE_F_BITS_AS_MULTIPLIER 65536

// phmod_proportion is an 1n15 fixed-point number only using
// the fractional part and the sign bit
#define SAMPLE_PHMOD_BITS 16

enum interpolation {INTERP_NONE, INTERP_LINEAR};

class AudioPlayMemorySample : public AudioStream
{
public:
	AudioPlayMemorySample(void) : AudioStream(0, NULL), playing(0), phase_incr(1), interpolation(false), pmod(0.5) { setLength(500); }
	void play(const unsigned int *data,unsigned int size);
	void play();
	void setSample(const unsigned int *data, unsigned int size);
	void stop(void);
	bool isPlaying(void) { return playing; }
	uint32_t positionMillis(void);
	uint32_t lengthMillis(void);
	uint32_t lengthBytes(void);
	void setSpeed(float);
	float getSpeed();
	virtual void update(void);
	long double getPhase(void);
	void pitchMod(float);
	void setLength(int32_t milliseconds)
	{
		if(milliseconds < 0)
			return;
		if(milliseconds > 5000)
			milliseconds = 5000;

		int32_t len_samples = milliseconds*(AUDIO_SAMPLE_RATE_EXACT/1000.0);

		env_decrement = (0x7fff0000/len_samples);
	};
//private:
	const unsigned int *next;
	const unsigned int *beginning;
	float length;
	float pmod;
	int16_t prior;
	uint32_t format;
	volatile uint8_t playing;
	unsigned int sampleSize;
	bool interpolation;
	
	// Envelope params
	int32_t env_lin_current; // present value of linear slope.
	int32_t env_decrement;   // how each sample deviates from previous.
	
	void parseSample();
	//std::vector <int16_t> buffer;
	unsigned int buffPost;
	
	long double phase;
	// volatile prevents the compiler optimizing out the frequency function
	volatile float phase_incr;
};

#endif
