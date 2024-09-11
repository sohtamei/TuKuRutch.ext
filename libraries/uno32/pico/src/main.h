#ifndef main_h
#define main_h

#if defined(ESP32)
  #include <TukurutchEsp.h>
  extern WebsocketsServer wsServer;
#endif

// ポート定義
#if defined(__AVR_ATmega328P__)
  #define P_BUZZER  12
#elif defined(ESP32)
  #define P_BUZZER   19
  #define P_BUZZER2  16
  #define P_BUZZER2G 14
  #define LEDC_BUZZER2 13
#elif defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
  #define P_BUZZER  17//16//19
  #define P_BUZZER2 18//20
#endif

void _setup(const char* ver);
void _loop(void);

uint16_t _getAdc1(uint8_t idx, uint16_t count, uint8_t discharge);
void _setLED(uint8_t idx, uint8_t onoff);
uint8_t _getSw(uint8_t idx);

void _tone(uint8_t port, int16_t freq, int16_t ms);
int _analogRead(uint8_t port, uint16_t count);

#if defined(ESP32) || defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
void _regHist();
void _saveHist();
void _setMelody(uint8_t* buf, int size);
#endif

extern uint8_t comMode;
void playbackPackets(uint8_t* buf, int size);
int getCurPacket(uint8_t* buf, int size);

#endif  // main_h
