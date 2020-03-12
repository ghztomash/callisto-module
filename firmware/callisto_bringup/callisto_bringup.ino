#include <Audio.h>
#include <EEPROM.h>
#include "vt100.h"

#define VERSION 1

// PIN defines
#define LEDA 0
#define LEDB 1
#define LEDC 2
#define LEDD 3
#define LEDE 4
#define LEDF 5
#define LEDTRIG 10

#define SWITCHA 8
#define SWITCHB 9

#define TRIGIN 11

#define CVA  A0
#define POTA A1
#define CVB  A2
#define POTB A3
#define CVC  A4
#define POTC A5
#define CVD  A6
#define POTD A7
#define CVE  A8
#define POTE A9
#define CVF  A10
#define POTF A11

// Audio Objects
AudioSynthWaveformSine			osc1;
AudioSynthWaveform				osc2;
AudioAnalyzeRMS      			rms1;
AudioOutputAnalog 				out1;
AudioConnection					patchCordrms(osc2, 0, rms1, 0);
AudioConnection					patchCordout(osc1, 0, out1, 0);

// Global Variables
int trig = 0;
int lastTrig = 0;

int currentModeA = 5;
int currentModeB = 0;

bool modeButtonA = LOW;
bool lastModeButtonA = LOW;
bool modeButtonB = LOW;
bool lastModeButtonB = LOW;

int ADC_RESOLUTION = 13;
int ADC_AVERAGING = 8;
int SAMPLES = 50;	// 5000

int MAXADC = (1 << ADC_RESOLUTION) - 1;	// maximum possible reading from ADC
float VREF = 3.3;		// ADC reference voltage (= power supply)
float VINPUT = 1.65;	// ADC input voltage from Normalized Jack      
int EXPECTED = MAXADC*(VINPUT/VREF);	// expected ADC reading
float VINPUTVOCT = 2.357;	// ADC input voltage from Normalized Jack 
int EXPECTEDVOCT = MAXADC*(VINPUTVOCT/VREF);	// expected ADC reading

int ledsPins[6] = {LEDA, LEDB, LEDC, LEDD, LEDE, LEDF};
int ledsStates[6] = {0};
int trigLedState = 0;

int adcPins[12] = {CVA, POTA, CVB, POTB, CVC, POTC, CVD, POTD, CVE, POTE, CVF, POTF};
int adcVal[12] = {0};
int adcCalibration[12] = {0}; // calibration table;
int adcOffset[12] = {0}; // offset table;
//uint32_t adcSum[12] = {0};

float frequency[] = {100, 1000, 10000};
float amplitude[] = {0.0, 0.5, 1.0};

char inString[124];

long oldT;
long durT;

enum CLI_COMMAND {
	CLI_ERR,
	CLI_REF,
	CLI_AMP,
	CLI_FREQ,
	CLI_CAL,
	CLI_SAVE,
	CLI_LOAD,
	CLI_SIZE,
	CLI_BIT,
	CLI_AVR,
	CLI_RES
};

struct EepromData {
	int config;
	int version;
	int calibration[12];
};

void setup() {

  for (int i=0; i<6; i++){
	pinMode(ledsPins[i], OUTPUT);
	digitalWrite(ledsPins[i], HIGH);
	delay(100);
  }
  for (int i=0; i<6; i++){
	digitalWrite(ledsPins[i], LOW);
	delay(100);
  }
  pinMode(LEDTRIG, OUTPUT);
  digitalWrite(LEDTRIG, HIGH);

  pinMode(TRIGIN, INPUT_PULLUP);
  pinMode(SWITCHA, INPUT_PULLUP);
  pinMode(SWITCHB, INPUT_PULLUP);
  
  digitalWrite(ledsPins[currentModeA], HIGH);
  digitalWrite(ledsPins[currentModeB], HIGH);

  analogReference(EXTERNAL);
  analogReadResolution(ADC_RESOLUTION);
  analogReadAveraging(ADC_AVERAGING);
  
  AudioMemory(64);
  //osc1.begin(WAVEFORM_SINE);
  osc1.frequency(frequency[0]);
  osc1.amplitude(amplitude[2]);
  osc2.begin(1.0, 0.25, WAVEFORM_SINE);
  
  inString[0]='\0';
  
  loadEeprom();
  
}

void loop() {
	
  int tab = 0;

  modeButtonA = digitalRead(SWITCHA);
  
  if(modeButtonA != lastModeButtonA){
	  if(modeButtonA == LOW){
		  digitalWrite(ledsPins[currentModeA], LOW);
		  currentModeA++;
		  if(currentModeA >= 6)
			  currentModeA = 3;
		  digitalWrite(ledsPins[currentModeA], HIGH);
		  osc1.amplitude(amplitude[currentModeA-3]);
	  }
	lastModeButtonA = modeButtonA;
  }
  
  modeButtonB = digitalRead(SWITCHB);
  
  if(modeButtonB != lastModeButtonB){
	  if(modeButtonB == LOW){
		  digitalWrite(ledsPins[currentModeB], LOW);
		  currentModeB++;
		  if(currentModeB >= 3)
			  currentModeB = 0;
		  digitalWrite(ledsPins[currentModeB], HIGH);
		  osc1.frequency(frequency[currentModeB]);
	  }
	lastModeButtonB = modeButtonB;
  }
  
  trig = digitalRead(TRIGIN);
  if(trig != lastTrig){
	  if(trig == LOW){
		  
	  }
	lastTrig = trig;
  }
  
  //printClear();
  //setCursor(1,1);
  
  //drawRow(1,1,30,205);
  //drawRow(1,44,30,205);
  //drawBox(TABLE_SETUP,1,74,4);
  setCursor(TABLE_SETUP+1,3);
  print("ADC Res: ");
  print(ADC_RESOLUTION);
  print("-bit");
  setCursor(TABLE_SETUP+1,26);
  print("ADC Avrg: ");
  print(ADC_AVERAGING);
  setCursor(TABLE_SETUP+1,50);
  print("ADC max: ");
  print(MAXADC);
  
  setCursor(TABLE_SETUP+2,3);
  eraseLine();
  print("Stat Size: ");
  print(SAMPLES);
  setCursor(TABLE_SETUP+2,26);
  print("ADC duration: ");
  print((int)durT);
  print("uS");
  setCursor(TABLE_SETUP+2,50);
  print("ModeA: ");
  print(currentModeA-3);
  
  setCursor(TABLE_SETUP+3,3);
  eraseLine();
  print("DAC Frequency: ");
  print(frequency[currentModeB],0);
  print("Hz");
  setCursor(TABLE_SETUP+3,26);
  print("DAC Amplitude: ");
  print(amplitude[currentModeA-3]);
  setCursor(TABLE_SETUP+3,50);
  print("ModeB: ");
  print(currentModeB);
  
  setCursor(TABLE_SETUP+4,3);
  eraseLine();
  print("Trigger: ");
  print(HIGHLOW(!trig));
  setCursor(TABLE_SETUP+4,26);
  print("Switch A: ");
  print(HIGHLOW(!modeButtonA));
  setCursor(TABLE_SETUP+4,50);
  print("Switch B: ");
  print(HIGHLOW(!modeButtonB));
  
  //drawBox(TABLE_SETUP,1,74,4);
  //drawBox(TABLE_ADC,1,74,13);
  //drawBox(TABLE_GRAPH,1,74,12);
  if(!DRAWCSV){
	  tab=3;
	  setCursor(TABLE_ADC+1,tab);
	  printColor(VTBOLD,VTWHITE, VTBLACK);
	  print("PIN");
	  tab+=TAB_INCREMENT;
	  setCursor(TABLE_ADC+1,tab);
	  print("ADC");
	  tab+=TAB_INCREMENT;
	  setCursor(TABLE_ADC+1,tab);
	  print("ADC V");
	  tab+=TAB_INCREMENT;
	  setCursor(TABLE_ADC+1,tab);
	  print(" VIN");
	  tab+=TAB_INCREMENT;
	  setCursor(TABLE_ADC+1,tab);
	  print("VAR");
	  tab+=TAB_INCREMENT;
	  setCursor(TABLE_ADC+1,tab);
	  print("STD");
	  tab+=TAB_INCREMENT;
	  setCursor(TABLE_ADC+1,tab);
	  print("AVG");
	  tab+=TAB_INCREMENT;
	  setCursor(TABLE_ADC+1,tab);
	  print("PPN");
	  tab+=TAB_INCREMENT;
	  setCursor(TABLE_ADC+1,tab);
	  print("OFF");
  } else {
	  tab=3;
	  setCursor(TABLE_ADC+1,tab);
	  resetColor();
	  print("PIN, ");
	  tab+=TAB_INCREMENT;
	  setCursor(TABLE_ADC+1,tab);
	  print("ADC, ");
	  tab+=TAB_INCREMENT;
	  setCursor(TABLE_ADC+1,tab);
	  print("VAR, ");
	  tab+=TAB_INCREMENT;
	  setCursor(TABLE_ADC+1,tab);
	  print("STD, ");
	  tab+=TAB_INCREMENT;
	  setCursor(TABLE_ADC+1,tab);
	  print("AVG, ");
	  tab+=TAB_INCREMENT;
	  setCursor(TABLE_ADC+1,tab);
	  print("PPN");
  }
  //cursorForward(6);
  resetColor();
  println();
  
	float maxVariance = 0;
	float maxSTD = 0;
	int maxPPN = 0;
	float avrVariance = 0;
	float avrSTD = 0;
	int avrPPN = 0;

  for (int i=0; i<12; i++ ){
	  
	  delay(5);
	  
	  oldT = micros();
	  adcVal[i] = analogRead(adcPins[i]) - adcCalibration[i]; // callibration offset
	  durT = micros() - oldT;
	  
	  long datSum = 0;  // reset our accumulated sum of input values to zero
      int sMax = 0;
      int sMin = MAXADC;
      long n;            // count of how many readings so far
      double x,mean,delta,m2,variance,stdev;  // to calculate standard deviation
	  
      n = 0;     // have not made any ADC readings yet
      mean = 0; // start off with running mean at zero
      m2 = 0;
	  
	  
	  for (int j=0; j<SAMPLES; j++) {
        x = analogRead(adcPins[i]) - adcCalibration[i]; // callibration offset
        datSum += x;
        if (x > sMax) sMax = x;
        if (x < sMin) sMin = x;
        n++;
        delta = x - mean;
        mean += delta/n;
        m2 += (delta * (x - mean));
      }
	  
	  variance = m2/(n-1);  // (n-1):Sample Variance  (n): Population Variance
      stdev = sqrt(variance);  // Calculate standard deviation
	  float datAvg = (1.0*datSum)/n;
	  int ppNoise = sMax - sMin;
	  
	  if (ppNoise > maxPPN)
		maxPPN = ppNoise;
	  if ( stdev > maxSTD)
		maxSTD = stdev;
	  if (variance > maxVariance)
		  maxVariance = variance;
	  avrPPN += ppNoise;
	  avrSTD +=stdev;
	  avrVariance +=variance;
	  
	  int sOffset;
	  
	  if(i==0) // for 1v/oct input
		sOffset = datAvg - EXPECTEDVOCT;
	  else
		sOffset = datAvg - EXPECTED;
	
	  adcOffset[i] = sOffset;
	  
	  if(i%2==0)
		printColor(VTNORMAL,VTYELLOW,VTBLACK);
	  else
		resetColor();
	  if (!DRAWCSV) {
		  tab=3;
		  setCursor(TABLE_ADC+2+i, tab);
		  eraseLine();
		  print("A");
		  print(i);
		  print(":");
		  tab+=TAB_INCREMENT;
		  setCursor(TABLE_ADC+2+i, tab);
		  print(adcVal[i]);
		  tab+=TAB_INCREMENT;
		  setCursor(TABLE_ADC+2+i, tab);
		  print((adcVal[i]/(float)MAXADC)*3.3, 3);
		  tab+=TAB_INCREMENT;
		  setCursor(TABLE_ADC+2+i, tab);
		  if(i==0) // for 1v/oct input
			print(((adcVal[i]/(float)MAXADC)*3.3)/-0.33+7.14, 3, 1);
		  else if(i%2==0)
			print(((adcVal[i]/(float)MAXADC)*3.3)/-0.33+5, 3, 1);
		  tab+=TAB_INCREMENT;
		  setCursor(TABLE_ADC+2+i, tab);
		  print((float)variance,1);
		  tab+=TAB_INCREMENT;
		  setCursor(TABLE_ADC+2+i, tab);
		  print((float)stdev);
		  tab+=TAB_INCREMENT;
		  setCursor(TABLE_ADC+2+i, tab);
		  print((float)datAvg,1);
		  tab+=TAB_INCREMENT;
		  setCursor(TABLE_ADC+2+i, tab);
		  print((int)ppNoise);
		  tab+=TAB_INCREMENT;
		  setCursor(TABLE_ADC+2+i, tab);
		  if(i%2==0)
			print((int)sOffset);
		  
		  tab=3;
		  setCursor(TABLE_GRAPH+1+i, tab);
		  eraseLine();
		  print("A");
		  print(i);
		  print(":");
		  
		  drawRow(TABLE_GRAPH+1+i, 8,(adcVal[i]>>7), 219 );
	  } else {
		  tab=3;
		  setCursor(TABLE_ADC+2+i, tab);
		  eraseLine();
		  print("A");
		  print(i);
		  print(":, ");
		  tab+=TAB_INCREMENT;
		  setCursor(TABLE_ADC+2+i, tab);
		  print(adcVal[i]);
		  print(", ");
		  tab+=TAB_INCREMENT;
		  setCursor(TABLE_ADC+2+i, tab);
		  print((float)variance,1);
		  print(", ");
		  tab+=TAB_INCREMENT;
		  setCursor(TABLE_ADC+2+i, tab);
		  print((float)stdev);
		  print(", ");
		  tab+=TAB_INCREMENT;
		  setCursor(TABLE_ADC+2+i, tab);
		  print((float)datAvg,1);
		  print(", ");
		  tab+=TAB_INCREMENT;
		  setCursor(TABLE_ADC+2+i, tab);
		  print((int)ppNoise);
		  print("\n");
	  }
  }
  
  // print stats for all ADC pins
  avrPPN /= 12;
  avrSTD /= 12.0;
  avrVariance /= 12.0;
  
  setCursor(TABLE_GRAPH-2, 3 + TAB_INCREMENT * 3);
  eraseLine();
  print("Avr:");
  setCursor(TABLE_GRAPH-1, 3 + TAB_INCREMENT * 3);
  eraseLine();
  print("Max:");
  printColor(VTNORMAL,VTCYAN,VTBLACK);
  setCursor(TABLE_GRAPH-1, 3 + TAB_INCREMENT * 4);
  print((float)maxVariance, 1);
  setCursor(TABLE_GRAPH-1, 3 + TAB_INCREMENT * 5);
  print((float)maxSTD);
  setCursor(TABLE_GRAPH-1, 3 + TAB_INCREMENT * 7);
  print(maxPPN);
  
  setCursor(TABLE_GRAPH-2, 3 + TAB_INCREMENT * 4);
  print((float)avrVariance, 1);
  setCursor(TABLE_GRAPH-2, 3 + TAB_INCREMENT * 5);
  print((float)avrSTD);
  setCursor(TABLE_GRAPH-2, 3 + TAB_INCREMENT * 7);
  print(avrPPN);
  
  resetColor();
  
  //print CSV values
  
  //drawBox(TABLE_SETUP,1,74,4);
  //drawBox(TABLE_ADC,1,74,13);
  //drawBox(TABLE_GRAPH,1,74,12);
  
  setCursor(TABLE_SETUP,30);
  //eraseLine();
  print("-<");
  printColor(VTBOLD,random(1,7),VTBLACK);
  print(" CALLISTO "); //14
  resetColor();
  print(">-");
  
  setCursor(TABLE_GRAPH+14,2);
  eraseLine();
	
  if(rms1.available()){
	float rms = rms1.read();
	print("Trig LED: ");
	print(rms* 255, 0);
	analogWrite(LEDTRIG, rms * 255);
  }
  
  setCursor(TABLE_GRAPH+14,26);
  print("Reference ADC: ");
  print(VINPUT);
  print("V\t");
  print(VINPUT/-0.33+5,2,1);
  print("V\t");
  print(EXPECTED);
  
  println();
  
  if(strlen(inString)>1){
	print(">>> ");
	print(inString);
  }
  
  while(Serial.available()){
	char inByte = Serial.read();
	if(inByte == '\n'){
		parseCommand();
		inString[0]='\0';
	} else {
		sprintf(inString, "%s%c", inString, inByte);
	}
  }
  
  setCursor(TABLE_GRAPH+16,26);
  print("Bring up firmware 1v by Tomash GHz");
  
  printWrite();
  
  delay(10);
  
}

float linToExp(float index, float range){
  //convert linear values to exponential
  float ratio = pow(range, 1/(float)range);
  return pow(ratio,index);
}

void parseCommand(){
	char * pch;
	pch = strtok (inString," ,");
	//println();
	uint8_t command = CLI_COMMAND::CLI_ERR;
	uint8_t arguments = 0;
	
	while (pch != NULL)
	{
		//print(pch);
		
		if (strncmp (pch,"ref",3) == 0)
		{
		  print(" found\n");
		  command = CLI_COMMAND::CLI_REF;
		} else if (strncmp (pch,"amp",3) == 0)
		{
		  print(" found\n");
		  command = CLI_COMMAND::CLI_AMP;
		} else if (strncmp (pch,"freq",3) == 0)
		{
		  print(" found\n");
		  command = CLI_COMMAND::CLI_FREQ;
		} else if (strncmp (pch,"cal",3) == 0)
		{
		  print(" found\n");
		  command = CLI_COMMAND::CLI_CAL;
		  // callibrate 
		  for (int i=0; i<12; i++ ){
			if(i%2==0){
				adcCalibration[i] = adcOffset[i];
			}
		  }
		}
		else if (strncmp (pch,"save",3) == 0)
		{
		  print(" found\n");
		  command = CLI_COMMAND::CLI_SAVE;
		  saveEeprom();
		} else if (strncmp (pch,"load",3) == 0)
		{
		  print(" found\n");
		  command = CLI_COMMAND::CLI_LOAD;
		  loadEeprom();
		} else if (strncmp (pch,"size",3) == 0)
		{
		  print(" found\n");
		  command = CLI_COMMAND::CLI_SIZE;
		} else if (strncmp (pch,"bit",3) == 0)
		{
		  print(" found\n");
		  command = CLI_COMMAND::CLI_BIT;
		} else if (strncmp (pch,"avr",3) == 0)
		{
		  print(" found\n");
		  command = CLI_COMMAND::CLI_AVR;
		} else if (strncmp (pch,"res",3) == 0)
		{
		  print(" found\n");
		  command = CLI_COMMAND::CLI_RES;
		  
		  for (int i=0; i<12; i++ ){
				adcCalibration[i] = 0;
		  }
		}
		
		if (arguments == 1){
			if(command == CLI_COMMAND::CLI_REF){
				float arg = atof(pch);
				if (arg>12.0)
					arg = 12.0;
				if (arg<-12.0)
					arg = -12.0;
				
				VINPUT = (arg*-0.33)+1.65;
				EXPECTED = MAXADC*(VINPUT/VREF);	// expected ADC reading
				
				VINPUTVOCT = (arg*-0.33)+2.357;
				EXPECTEDVOCT = MAXADC*(VINPUTVOCT/VREF);	// expected ADC reading
			} else if(command == CLI_COMMAND::CLI_AMP){
				float arg = atof(pch);
				if (arg>1.0)
					arg = 1.0;
				if (arg<0.0)
					arg = 0.0;
				
				amplitude[currentModeA-3] = arg;
				osc1.amplitude(amplitude[currentModeA-3]);
			} else if(command == CLI_COMMAND::CLI_FREQ){
				float arg = atof(pch);
				if (arg>20000.0)
					arg = 20000.0;
				if (arg<1.0)
					arg = 1.0;
				
				frequency[currentModeB] = arg;
				osc1.frequency(frequency[currentModeB]);
			} else if(command == CLI_COMMAND::CLI_SIZE){
				float arg = atof(pch);
				if (arg>5000.0)
					arg = 5000.0;
				if (arg<1.0)
					arg = 1.0;
				
				SAMPLES = arg;
			} else if(command == CLI_COMMAND::CLI_BIT){
				float arg = atof(pch);
				if (arg>13.0)
					arg = 13.0;
				if (arg<1.0)
					arg = 1.0;
				
				ADC_RESOLUTION = arg;
				MAXADC = (1 << ADC_RESOLUTION) - 1;
				analogReadResolution(ADC_RESOLUTION);
				
				VINPUT = (arg*-0.33)+1.65;
				EXPECTED = MAXADC*(VINPUT/VREF);	// expected ADC reading
				VINPUTVOCT = (arg*-0.33)+2.357;
				EXPECTEDVOCT = MAXADC*(VINPUTVOCT/VREF);
			} else if(command == CLI_COMMAND::CLI_AVR){
				float arg = atof(pch);
				if (arg>32.0)
					arg = 32.0;
				if (arg<1.0)
					arg = 1.0;
				
				ADC_AVERAGING = arg;
				analogReadAveraging(ADC_AVERAGING);
			}
		} 
		
		pch = strtok (NULL, " ,");
		arguments++;
	}
	//println();
}

void saveEeprom(){
	int eeAddress = 0;
	EepromData eedata = { 0xDEAD, VERSION, {0}};
	
	for (int i=0; i<12; i++ ){
		eedata.calibration[i] = adcCalibration[i];
	}
	
	EEPROM.put(eeAddress, eedata);
}

void loadEeprom(){
	int eeAddress = 0;
	EepromData eedata;
	EEPROM.get(eeAddress, eedata);
	
	if(eedata.config == 0xDEAD){
		print("eeprom data\n");
		for (int i=0; i<12; i++ ){
			adcCalibration[i] = eedata.calibration[i];
		}
	} else {
		print("no data\n");
	}
}