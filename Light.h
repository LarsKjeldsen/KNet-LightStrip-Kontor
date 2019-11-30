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

void SetBrightness(int, int);
void PowerON(int, int);
void PowerOFF(int, int);
void SetupLeds();
void SetLight(int Mode_top, int Brightness_top, int Mode_But, int Brightness_But);

#endif

