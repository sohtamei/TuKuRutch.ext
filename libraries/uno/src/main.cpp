/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include "main.h"

enum {
	T_C4=262, T_D4=294, T_E4=330, T_F4=349, T_G4=392, T_A4=440, T_B4=494,
	T_C5=523, T_D5=587, T_E5=659, T_F5=698,
};

// ポート定義

#define P_GND		A1

#define numof(a) (sizeof(a)/sizeof((a)[0]))

struct port {uint8_t sig; uint8_t gnd;};

const uint8_t sensorTable[4] = {A2, A3, A4, A5};
uint16_t _getAdc1(uint8_t idx, uint16_t count, uint8_t discharge)
{
	if(!idx || idx > numof(sensorTable)) return 0;

	uint8_t ch = sensorTable[idx-1];
	if(discharge) {
		digitalWrite(ch, LOW);
		pinMode(ch, OUTPUT);		// 電荷discharge (リモコンロボで1000mVくらいに帯電してしまう)
		delay(1);
		pinMode(ch, INPUT);
	}
	if(count == 0) count = 1;
	count *= 100;
	uint32_t sum = 0;
	for(int i = 0; i < count; i++)
		sum += analogRead(ch);
	sum = ((sum / count) * 625UL) / 128;  // 1024->5000
	return sum;
}

const struct port ledTable[6] = {{13,0}, {2,3}, {4,5}, {6,7}, {8,9}, {10,11}};
void _setLED(uint8_t idx, uint8_t onoff)
{
	if(!idx || idx > numof(ledTable)) return;
	idx--;

	digitalWrite(ledTable[idx].sig, onoff);
	pinMode(ledTable[idx].sig, OUTPUT);
	if(ledTable[idx].gnd) {
		digitalWrite(ledTable[idx].gnd, LOW);
		pinMode(ledTable[idx].gnd, OUTPUT);
	}
}

const struct port swTable[3] = {{2,4},{5,7},{8,10}};
uint8_t _getSw(uint8_t idx)
{
	if(!idx || idx > numof(swTable)) return 0;
	idx--;

	pinMode(swTable[idx].sig, INPUT_PULLUP);
	if(swTable[idx].gnd) {
		digitalWrite(swTable[idx].gnd, LOW);
		pinMode(swTable[idx].gnd, OUTPUT);
	}
	return digitalRead(swTable[idx].sig) ? 0: 1;
}

void _setup(const char* ver)
{
	_setLED(1,0);
	pinMode(P_GND, OUTPUT);
	digitalWrite(P_GND, LOW);

	pinMode(P_BUZZER, OUTPUT);
	_tone(P_BUZZER, T_C5, 100);
	Serial.begin(115200);
}

void _loop(void)
{
}
