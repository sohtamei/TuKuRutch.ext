/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include <TukurutchEsp.h>
#include "main.h"

// ポート定義

WebsocketsServer wsServer;

#define P_GND		4

#define P_BUZZER		19
#define LEDC_BUZZER		8

#define numof(a) (sizeof(a)/sizeof((a)[0]))

struct port {uint8_t sig; uint8_t gnd;};

const uint8_t sensorTable[4] = {35, 34, 36, 39};
uint16_t _getAdc1(uint8_t idx, uint16_t count)
{
	if(!idx || idx > numof(sensorTable)) return 0;
	return getAdc1(sensorTable[idx-1], count);
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

void _tone(int sound, int ms)
{
	ledcWriteTone(LEDC_BUZZER, sound);
	delay(ms);
	ledcWriteTone(LEDC_BUZZER, 0);
}

void onConnect(String ip)
{
	_setLED(1,1);
	_tone(T_C4, 250);
	_tone(T_D4, 250);
	_tone(T_E4, 250);

	wsServer.listen(PORT_WEBSOCKET);
	Serial.println(ip);
}

void _setup(const char* ver)
{
	int i;
	_setLED(1,0);
	pinMode(P_GND, OUTPUT);
	digitalWrite(P_GND, LOW);
	ledcSetup(LEDC_BUZZER, 5000/*Hz*/, 13/*bit*/);
	ledcAttachPin(P_BUZZER, LEDC_BUZZER);

	_tone(T_C5, 100);
	Serial.begin(115200);
	if(_getSw(1)) {
		delay(100);
		_tone(T_C5, 100);
		Serial.println("Waiting for SmartConfig.");
		WiFi.mode(WIFI_STA);
		WiFi.beginSmartConfig();
		while (!WiFi.smartConfigDone()) {
			delay(1000);
			_setLED(1,1);
			_tone(T_C5, 100);
			_setLED(1,0);
		}
		Serial.println("SmartConfig received.");
	}
	initWifi(ver, false, onConnect);
}

void _loop(void)
{
//	loopWebSocket();
//	delay(50);
}
