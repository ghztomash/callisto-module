
#ifndef play_memory_s_h_
#define play_memory_s_h_

#include "AudioStream.h"
#include "Arduino.h"

//enum Interpolation {INTERP_NONE, INTERP_LINEAR};

class AudioPlayMemorySample : public AudioStream
{
public:
	AudioPlayMemorySample(void) : AudioStream(1, inputQueueArray), playing(0), phase_incr(1) { }
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
	
//private:
	const unsigned int *next;
	const unsigned int *beginning;
	float length;
	uint32_t format;
	volatile uint8_t playing;
	uint32_t sampleSize;

	audio_block_t *inputQueueArray[1];
	
	uint32_t phase;
	// volatile prevents the compiler optimizing out the frequency function
	volatile int32_t phase_incr;
};

#endif
