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
  #define P_BUZZER  19
#elif defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
  #define P_BUZZER  19
#endif

void _setup(const char* ver);
void _loop(void);

uint16_t _getAdc1(uint8_t idx, uint16_t count, uint8_t discharge);
void _setLED(uint8_t idx, uint8_t onoff);
uint8_t _getSw(uint8_t idx);

void _tone(uint8_t port, int16_t freq, int16_t ms);
int _analogRead(uint8_t port, uint16_t count);

#endif  // main_h