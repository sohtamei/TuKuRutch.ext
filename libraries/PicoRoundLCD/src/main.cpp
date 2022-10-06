/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include "main.h"


#define BL  10
LGFX_RPICO_SPI_GC9A01 lcd;

#define numof(a) (sizeof(a)/sizeof((a)[0]))

void _setLED(uint8_t onoff)
{;}

uint8_t _getSw(uint8_t button)
{
	return 0;
}

float getIMU(uint8_t index)
{
	return 0;
}

void _setup(const char* ver)
{
	Serial.begin(115200);

	pinMode(BL,OUTPUT);
	digitalWrite(BL,HIGH);

	lcd.init();

	lcd.setBrightness(128);
	lcd.fillScreen(TFT_BLACK);
	lcd.setTextSize(2);
/*
	lcd.width(), lcd.height()
	lcd.startWrite();
	lcd.endWrite();
*/

	lcd.setCursor(0, 0);
	lcd.println(ver);
}

void _loop(void)
{
//	delay(50);
}
