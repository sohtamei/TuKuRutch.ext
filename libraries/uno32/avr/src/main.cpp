/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include "main.h"


#if defined(ESP32) || defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
#include <Preferences.h>
Preferences preferencesHIST;  // cmdHistNVP, cmdHistMode
enum {
	CmdHistMode_NULL = 0,
	CmdHistMode_PACKETS,
	CmdHistMode_MIDI2,
};
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
  void setNeoPixel(int c, int level);
#endif

uint16_t _getAdc1(uint8_t idx, uint16_t count, uint8_t discharge)
{
	if(!idx || idx > numof(sensorTable)) return 0;

	uint8_t ch = sensorTable[idx-1];
#if !defined(ESP32)
	if(discharge) {
		pinMode(ch, OUTPUT);		// 電荷discharge (リモコンロボで1000mVくらいに帯電してしまう)
		digitalWrite(ch, LOW);
		delay(1);
		pinMode(ch, INPUT);
		delay(discharge);
	}
#endif
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
uint8_t cmdHistNVP[2048] = {0};
int cmdHistNVP_num = 0;
int cmdHistMode = CmdHistMode_NULL;

uint8_t cmdHist[1024] = {0};
int cmdHist_count = 0;
int cmdHist_last = 0;

void _regHist()
{
	if(comMode == 0) return;	// don't register MODE_INVALID

	uint32_t last = cmdHist_last;
	cmdHist_last = millis();
	if(((cmdHist_last - last) & 0x7FFFFFFF) > 4000) cmdHist_count = 0;;

	int size = getCurPacket(cmdHist+cmdHist_count, sizeof(cmdHist)-cmdHist_count);
	cmdHist_count += size;
}

void _saveHist()
{
	if(cmdHist_count != 0 && ((millis() - cmdHist_last) & 0x7FFFFFFF) < 4000) {
		if(cmdHistNVP_num != cmdHist_count || memcmp(cmdHistNVP, cmdHist, cmdHist_count)){
			cmdHistNVP_num = cmdHist_count;
			memcpy(cmdHistNVP, cmdHist, cmdHist_count);
			preferencesHIST.putBytes("cmdHistNVP", cmdHistNVP, cmdHistNVP_num);

			cmdHistMode = CmdHistMode_PACKETS;
			preferencesHIST.putUChar("cmdHistMode", cmdHistMode);
			Serial.println(cmdHistNVP_num);
		}
	}
#if defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
	setNeoPixel(0x000000, 0);
#endif
	cmdHist_count = 0;
	cmdHist_last = 0;
}

#define NOTE_NUM  2
void _setMelody(uint8_t* buf, int size)
{
	if(size > sizeof(cmdHistNVP)) size = sizeof(cmdHistNVP);
	cmdHistNVP_num = size;
	memcpy(cmdHistNVP, buf, cmdHistNVP_num);
	preferencesHIST.putBytes("cmdHistNVP", cmdHistNVP, cmdHistNVP_num);

	cmdHistMode = CmdHistMode_MIDI2;
	preferencesHIST.putUChar("cmdHistMode", cmdHistMode);
	Serial.println(cmdHistNVP_num);
}

void _tone2(int16_t freq)
{
#if defined(P_BUZZER2)
	uint8_t port = P_BUZZER2;
  #if defined(ESP32)
	ledcAttachPin(port, LEDC_BUZZER2);
	ledcWriteTone(LEDC_BUZZER2, freq);
  #elif defined(NRF51_SERIES) || defined(NRF52_SERIES)
	;
  #else
	tone(port, freq, 0);
  #endif
#endif
}

void _playbackMidi2()
{
#if defined(ESP32) && defined(P_BUZZER2)
	pinMode(P_BUZZER2G, OUTPUT);
	digitalWrite(P_BUZZER2G, LOW);
#endif
	for(int i = 0; i < cmdHistNVP_num; i += (1+NOTE_NUM)*2) {
		#define GetL16(a) (((a)[1]<<8) | (a)[0])
		int dur   = GetL16(cmdHistNVP+i+0);
		int note1 = GetL16(cmdHistNVP+i+2);
		int note2 = GetL16(cmdHistNVP+i+4);
	//	Serial.printf("%d %d %d\n", dur, (note1==0xffff)?-1:note1, (note2==0xffff)?-1:note2);
		if(note1 != 0xFFFF) _tone(P_BUZZER, note1, 0);
		if(note2 != 0xFFFF) _tone2(note2);
		delay(dur);
	}
	_tone(P_BUZZER, 0, 0);
	_tone2(0);
}
#endif // ESP32,RP2040

#if defined(ESP32)
void onConnect(String ip)
{
	_setLED(1,1);
	_tone(P_BUZZER, T_C4, 100);

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
	cmdHistMode    = preferencesHIST.getUChar("cmdHistMode");

	Serial.printf("NVP:%d,%d\n", cmdHistMode, cmdHistNVP_num);
#endif
}

int state = 1;
void _loop(void)
{
#if defined(ESP32) || defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
	if(comMode == 0) {			// when comMode = MODE_INVALID
		int level = _getAdc1(1,4,8);	// id, count, discharge
		if(state == 0) {
			if(level > 600) {
				switch(cmdHistMode) {
				case CmdHistMode_PACKETS:
					playbackPackets(cmdHistNVP, cmdHistNVP_num);
					break;
				case CmdHistMode_MIDI2:
					_playbackMidi2();
					break;
				}
			#if defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
				setNeoPixel(0x000000, 0);
			#endif
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
