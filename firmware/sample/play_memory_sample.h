
#ifndef play_memory_s_h_
#define play_memory_s_h_

#include "AudioStream.h"
#include "Arduino.h"

class AudioPlayMemorySample : public AudioStream
{
public:
	AudioPlayMemorySample(void) : AudioStream(1, inputQueueArray), playing(0), phase_incr(1), modulation_factor(0), interpolate(true) { }
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
	bool interpolate;

	audio_block_t *inputQueueArray[1];
	
	uint32_t phase;
};

#endif
