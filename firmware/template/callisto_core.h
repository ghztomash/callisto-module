#ifndef Callisto_h
#define Callisto_h

#include "Arduino.h"

// PIN defines
#define LEDA 0
#define LEDB 1
#define LEDC 2
#define LEDD 3
#define LEDE 4
#define LEDF 5
#define LEDTRIG 10

int ledsPins[6] = {LEDA, LEDB, LEDC, LEDD, LEDE, LEDF};

class CallistoCore{
	public:
		CallistoCore();
		void blink();
	
};

CallistoCore::CallistoCore(){
	for (int i=0; i<6; i++){
		pinMode(ledsPins[i], OUTPUT);
		digitalWrite(ledsPins[i], HIGH);
		delay(100);
	}
	pinMode(LEDTRIG, OUTPUT);
	digitalWrite(LEDTRIG, HIGH);
}

void CallistoCore::blink(){
	digitalWrite(LEDTRIG, HIGH);
	delay(200);
	digitalWrite(LEDTRIG, LOW);
	delay(200);
}

#endif