#ifndef main_h
#define main_h

#include <M5Core2.h>

#define LGFX_M5STACK_CORE2         // M5Stack Core2
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>
extern LGFX lcd;

// ポート定義
#define P_BUZZER	25

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);

void _setLED(uint8_t onoff);
uint8_t _getSw(uint8_t button);
float getIMU(uint8_t index);
void _tone(uint8_t port, int16_t freq, int16_t ms);

#endif  // main_h
