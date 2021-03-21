/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>
#include "main.h"
#include <Adafruit_Microbit.h>
Adafruit_Microbit_Matrix microbit;

// ポート定義

#define P_LED			10

void _setup(const char* ver)
{
	Serial.begin(115200);
	microbit.begin();

	pinMode(PIN_BUTTON_A, INPUT);
	pinMode(PIN_BUTTON_B, INPUT);

}

void _loop(void)
{
}

void _displayText(char* text)
{
	microbit.print(text);
	microbit.clear();
}

void _displayLed(uint32_t bitmap)
{
	int i;
	uint8_t buf[5];
	for(i = 0; i < 5; i++)
		buf[i] = (bitmap>>i*5) & 0x1F;
	microbit.show(buf);
}

void _getData(void)
{
	uint8_t buf[9]={0};
/*
	buf[0] = Accel.getX()>>8;
	buf[1] = Accel.getX();
	buf[2] = Accel.getY()>>8;
	buf[3] = Accel.getY();
	buf[4] = uBit.buttonA.isPressed()|(evtButtonA<<1);
	buf[5] = uBit.buttonB.isPressed()|(evtButtonB<<1);
	buf[6] = evtPort;
	buf[7] = evtGesture;
	buf[8] = evtTilt;
*/
	buf[4] = (digitalRead(PIN_BUTTON_A)^1);
	buf[5] = (digitalRead(PIN_BUTTON_B)^1);
	sendBin(buf,9);
}

//
