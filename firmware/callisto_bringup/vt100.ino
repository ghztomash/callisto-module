#include "helpers.h"

const int MSGMAX = 8192;
char msg[MSGMAX];
char msgTemp[2048];
int  msgLen=0;

void printClear(){
	msg[0] = '\0';
	msgLen = sprintf(msg, "\033[2J\033[?25l"); //\033[0;0H
}

void print(const char str[]){
	if(strlen(msg) + strlen(str) >= MSGMAX){
		printWrite();
	}
	msgLen += sprintf(msg, "%s%s", msg, str);
}

void print(const String &s){
	if(strlen(msg) + strlen(s.c_str()) >= MSGMAX){
		printWrite();
	}
	msgLen += sprintf(msg, "%s%s", msg, s.c_str());
}

void print(int num){
	int tLen = sprintf(msgTemp,"%d",num);
	if(strlen(msg) + tLen >= MSGMAX){
		printWrite();
	}
	msgLen += sprintf(msg, "%s%s", msg, msgTemp);
}

void print(float num){
	print(num ,2);
}

void print(float num, int d){
	int tLen = 0;
	
	switch (d){
		case 0:
			tLen = sprintf(msgTemp,"%.0f",num);
			break;
		case 1:
			tLen = sprintf(msgTemp,"%.1f",num);
			break;
		case 2:
			tLen = sprintf(msgTemp,"%.2f",num);
			break;
		case 3:
			tLen = sprintf(msgTemp,"%.3f",num);
			break;
		case 4:
			tLen = sprintf(msgTemp,"%.4f",num);
			break;
	}
	
	if(strlen(msg) + tLen >= MSGMAX){
		printWrite();
	}
	msgLen += sprintf(msg, "%s%s", msg, msgTemp);
}

void print(float num, int d, int s){
	
	if(s==0){
		print(num,d);
		return;
	}
	
	int tLen = 0;
	
	switch (d){
		case 0:
			tLen = sprintf(msgTemp,"%+.0f",num);
			break;
		case 1:
			tLen = sprintf(msgTemp,"%+.1f",num);
			break;
		case 2:
			tLen = sprintf(msgTemp,"%+.2f",num);
			break;
		case 3:
			tLen = sprintf(msgTemp,"%+.3f",num);
			break;
		case 4:
			tLen = sprintf(msgTemp,"%+.4f",num);
			break;
	}
	
	if(strlen(msg) + tLen >= MSGMAX){
		printWrite();
	}
	msgLen += sprintf(msg, "%s%s", msg, msgTemp);
}

void eraseLine(){
	int tLen = sprintf(msgTemp,"\033[2K");
	if(strlen(msg) + tLen >= MSGMAX){
		printWrite();
	}
	msgLen += sprintf(msg, "%s%s", msg, msgTemp);
}

void println(){
	int tLen = sprintf(msgTemp,"\n\033[1C");
	if(strlen(msg) + tLen >= MSGMAX){
		printWrite();
	}
	msgLen += sprintf(msg, "%s%s", msg, msgTemp);
	eraseLine();
}

void printWrite(){
	Serial.write(msg);
	msg[0] = '\0';
	msgLen = strlen(msg);
}

void printColor(int attr, int fore, int back){
	int tLen = sprintf(msgTemp,"\033[%d;%d;%dm",attr, (30+fore), (40+back));
	if(strlen(msg) + tLen >= MSGMAX){
		printWrite();
	}
	msgLen += sprintf(msg, "%s%s", msg, msgTemp);
}

void resetColor(){
	int tLen = sprintf(msgTemp,"\033[0;37;40m");
	if(strlen(msg) + tLen >= MSGMAX){
		printWrite();
	}
	msgLen += sprintf(msg, "%s%s", msg, msgTemp);
}

void setCursor(int x, int y){
	int tLen = sprintf(msgTemp,"\033[%d;%dH", x, y);
	if(strlen(msg) + tLen >= MSGMAX){
		printWrite();
	}
	msgLen += sprintf(msg, "%s%s", msg, msgTemp);
}

void cursorForward(int x){
	int tLen = sprintf(msgTemp,"\033[%dC",x);
	if(strlen(msg) + tLen >= MSGMAX){
		printWrite();
	}
	msgLen += sprintf(msg, "%s%s", msg, msgTemp);
}

void drawRow(int x, int y, int r, char c){
	setCursor(x, y);
	msgTemp[0] = '\0';
	int tLen = strlen(msgTemp);
	for(int i = 0; i<r; i++){
		tLen += sprintf(msgTemp,"%s%c", msgTemp, char(c));
	}
	
	if(strlen(msg) + tLen >= MSGMAX){
		printWrite();
	}
	msgLen += sprintf(msg, "%s%s", msg, msgTemp);
}

void drawCol(int x, int y, int r, char c){
	for(int i = 0; i<r; i++){
		drawRow(x+i, y, 1, c);
	}
}

void drawBox(int x, int y, int w, int h){
	drawRow(x, y+1, w, 205);
	drawRow(x+h+1, y+1, w, 205);
	drawCol(x+1, y, h, 186);
	drawCol(x+1, y+w+1, h, 186);
	drawRow(x, y, 1, 201);
	drawRow(x+h+1, y, 1, 200);
	drawRow(x, y+w+1, 1, 187);
	drawRow(x+h+1, y+w+1, 1, 188);
}