/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include "main.h"


#if defined(ESP32) || defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
#include <Preferences.h>
Preferences preferencesHIST;
#endif

#if defined(ESP32)
  WebsocketsServer wsServer;
#else
  enum {
    T_C4=262, T_D4=294, T_E4=330, T_F4=349, T_G4=392, T_A4=440, T_B4=494,
    T_C5=523, T_D5=587, T_E5=659, T_F5=698,
  };
#endif

#define numof(a) (sizeof(a)/sizeof((a)[0]))
struct port {uint8_t sig; uint8_t gnd;};

// ポート定義

#if defined(__AVR_ATmega328P__)
  #define P_GND  A1
  const uint8_t sensorTable[] = {A2, A3, A4, A5};
  const struct port ledTable[] = {{13,0}, {2,3}, {4,5}, {6,7}, {8,9}, {10,11}};
  const struct port swTable[] = {{2,4},{5,7},{8,10}};
#elif defined(ESP32)
  #define P_GND  4
  const uint8_t sensorTable[] = {35, 34, 36, 39};
  const struct port ledTable[] = {{2,0}, {26,25}, {17,16}, {27,14}, {12,13}, {5,23}};
  const struct port swTable[] = {{26,17},{16,14},{12,5}};
#elif defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
  // port 16がダメ
  #define P_GND  0
  const uint8_t sensorTable[] = {27, 26, 28};
  const struct port ledTable[] = {{25,0}, {20,21},};
  const struct port swTable[] = {{24,0},/*{16,14},{12,5}*/};
#endif

uint16_t _getAdc1(uint8_t idx, uint16_t count, uint8_t discharge)
{
	if(!idx || idx > numof(sensorTable)) return 0;

	uint8_t ch = sensorTable[idx-1];
	if(discharge) {
		pinMode(ch, OUTPUT);		// 電荷discharge (リモコンロボで1000mVくらいに帯電してしまう)
		digitalWrite(ch, LOW);
		delay(1);
		pinMode(ch, INPUT);
		delay(discharge);
	}
	return _analogRead(ch, count);
}

void _setLED(uint8_t idx, uint8_t onoff)
{
	if(!idx || idx > numof(ledTable)) return;
	idx--;

	pinMode(ledTable[idx].sig, OUTPUT);
	digitalWrite(ledTable[idx].sig, onoff);
	if(ledTable[idx].gnd) {
		pinMode(ledTable[idx].gnd, OUTPUT);
		digitalWrite(ledTable[idx].gnd, LOW);
	}
}

uint8_t _getSw(uint8_t idx)
{
	if(!idx || idx > numof(swTable)) return 0;
	idx--;

	pinMode(swTable[idx].sig, INPUT_PULLUP);
	if(swTable[idx].gnd) {
		pinMode(swTable[idx].gnd, OUTPUT);
		digitalWrite(swTable[idx].gnd, LOW);
	}
	return digitalRead(swTable[idx].sig) ? 0: 1;
}

#if defined(ESP32) || defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
uint8_t cmdHistNVP[512] = {0};
int cmdHistNVP_num = 0;

uint8_t cmdHist[512] = {0};
int cmdHist_count = 0;
int cmdHist_last = 0;

void _regHist()
{
	if(comMode == 0) return;	// don't register MODE_INVALID

	uint32_t last = cmdHist_last;
	cmdHist_last = millis();
	if(((cmdHist_last - last) & 0x7FFFFFFF) > 1000) cmdHist_count = 0;;

	int size = getCurPacket(cmdHist+cmdHist_count, sizeof(cmdHist)-cmdHist_count);
	cmdHist_count += size;
}

void _saveHist()
{
	if(cmdHist_count == 0 || ((millis() - cmdHist_last) & 0x7FFFFFFF) < 1000) {
		if(cmdHistNVP_num != cmdHist_count || memcmp(cmdHistNVP, cmdHist, cmdHist_count)){
			cmdHistNVP_num = cmdHist_count;
			memcpy(cmdHistNVP, cmdHist, cmdHist_count);
			preferencesHIST.putBytes("cmdHistNVP", cmdHistNVP, cmdHistNVP_num);
			Serial.println(cmdHistNVP_num);
		}
	}
	cmdHist_count = 0;
	cmdHist_last = 0;
}
#endif // ESP32,RP2040

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
	Serial.begin(115200);
	_setLED(1,0);
	if(P_GND) {
		pinMode(P_GND, OUTPUT);
		digitalWrite(P_GND, LOW);
	}
#if defined(__AVR_ATmega328P__)
	pinMode(P_BUZZER, OUTPUT);
#endif
	_tone(P_BUZZER, T_C5, 100);

#if defined(ESP32)
	initWifi(ver, false, onConnect);
#endif

#if defined(ESP32) || defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
	preferencesHIST.begin("histConfig", false);
	cmdHistNVP_num = preferencesHIST.getBytes("cmdHistNVP", cmdHistNVP, sizeof(cmdHistNVP));
	Serial.println(cmdHistNVP_num);
#endif
}

int state = 0;
void _loop(void)
{
#if defined(ESP32) || defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
	if(comMode == 0) {			// when comMode = MODE_INVALID
		int level = _getAdc1(1,4,4);
		if(state == 0) {
			if(level > 600) {
				playbackPackets(cmdHistNVP, cmdHistNVP_num);
				state = 1;
			}
		} else {
			if(level < 500) {
				state = 0;
			}
		}
	}
#endif
}
