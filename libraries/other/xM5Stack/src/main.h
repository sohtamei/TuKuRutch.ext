#ifndef main_h
#define main_h

#define M5STACK_MPU6886 
// #define M5STACK_MPU9250 
#include <M5Stack.h>

#define LGFX_M5STACK               // M5Stack (Basic / Gray / Go / Fire)
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
