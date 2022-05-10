#include <Light.h>
#include "FastLED.h"


#define NUM_LEDS 120

#define BUT_S 0
#define BUT_E 43
#define TOP_S 45
#define TOP_E 89

#define DATA_PIN 2 //D5

CRGBArray<NUM_LEDS> leds;


void SetupLeds()
{
	Serial.println(F("Initialise LEDS - "));
	LEDS.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}


void SetLight(int Mode_top, int Brightness_top, int Mode_But, int Brightness_But)
{
	/*
		static uint8_t hue = 0;


		const CRGB cr = CRGB(255, 0, 0);
		const CRGB cg = CRGB(0, 255, 0);
		const CRGB cb = CRGB(0, 0, 255);

		leds(TOP_S, TOP_E).fill_solid(CHSV(255,255, Brightness_top));
		//	leds(40, 41).fill_solid(cg);
		//	leds(42, 43).fill_solid(cb);

		leds(44, 45).fill_solid(CRGB(0, 0, 0));


		leds(BUT_S, BUT_E).fill_rainbow(hue++);

		FastLED.delay(3);
	*/
}

void SetBrightness(int TopBut, int b)
{
	PowerON(TopBut, b);
}


void PowerOFF(int TopBut, int b)
{
	if (TopBut)
		for (int i = 0; i <= (TOP_E - TOP_S) / 2; i++) {
			leds[TOP_S + i] = CHSV(255, 0, 0);
			leds[TOP_E - i] = CHSV(255, 0, 0);
			LEDS.show();
			LEDS.delay(20);
		}
	else
		for (int i = 0; i <= (BUT_E - BUT_S) / 2; i++) {
			leds[BUT_S + i] = CHSV(128, 0, 0);
			leds[BUT_E - i] = CHSV(128, 0, 0);
			LEDS.show();
			LEDS.delay(20);
		}
}


void PowerON(int TopBut, int b)
{
	if (TopBut)
		for (int i = 0; i <= (TOP_E - TOP_S) / 2; i++) {
			leds[TOP_S + i] = CHSV(20, 60 + (i * 4), b);
			leds[TOP_E - i] = CHSV(20, 60 + (i * 4), b);
			LEDS.show();
			LEDS.delay(20);
		}
	else
		for (int i = 0; i <= (BUT_E - BUT_S) / 2; i++) {
			leds[BUT_S + i] = CHSV(0, 60 + (i * (b / 50)), b);
			leds[BUT_E - i] = CHSV(0, 60 + (i * (b / 50)), b);
			LEDS.show();
			LEDS.delay(20);
		}

//	LEDS.setBrightness(b);
//	LEDS.setTemperature(Tungsten40W);
	LEDS.show();
}

