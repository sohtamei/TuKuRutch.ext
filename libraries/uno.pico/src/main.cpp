/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include "main.h"

#if defined(ESP32)
  WebsocketsServer wsServer;
#else
  enum {
    T_C4=262, T_D4=294, T_E4=330, T_F4=349, T_G4=392, T_A4=440, T_B4=494,
    T_C5=523, T_D5=587, T_E5=659, T_F5=698,
  };
#endif

// ポート定義

#define P_GND		4

#define numof(a) (sizeof(a)/sizeof((a)[0]))

struct port {uint8_t sig; uint8_t gnd;};

const uint8_t sensorTable[4] = {35, 34, 36, 39};
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
	return _analogRead(ch, count);
}

const struct port ledTable[6] = {{2,0}, {26,25}, {17,16}, {27,14}, {12,13}, {5,23}};
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

const struct port swTable[3] = {{26,17},{16,14},{12,5}};
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

#if defined(ESP32)
void onConnect(String ip)
{
	_setLED(1,1);
	_tone(P_BUZZER, T_C4, 250);
	_tone(P_BUZZER, T_D4, 250);
	_tone(P_BUZZER, T_E4, 250);

	wsServer.listen(PORT_WEBSOCKET);
	Serial.println(ip);
}
#endif

void _setup(const char* ver)
{
	_setLED(1,0);
	pinMode(P_GND, OUTPUT);
	digitalWrite(P_GND, LOW);

	_tone(P_BUZZER, T_C5, 100);
	Serial.begin(115200);
#if defined(ESP32)
	initWifi(ver, false, onConnect);
#endif
}

void _loop(void)
{
}
