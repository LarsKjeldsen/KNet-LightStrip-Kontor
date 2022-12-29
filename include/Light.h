// Light.h

#ifndef _LIGHT_h
#define _LIGHT_h

#define FASTLED_INTERNAL 

#define BUT false
#define TOP true

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

bool SetBrightness(int, int);
bool PowerON(int, int);
bool PowerOFF(int, int);
void SetupLeds();
void SetLight(int Mode_top, int Brightness_top, int Mode_But, int Brightness_But);
void SendStatus();

#endif

