#ifndef main_h
#define main_h

#define _M5StickC
//#define _M5StickCPlus

#ifdef _M5StickC
  #define mVersion "M5StickC 1.0"
  #include <M5StickC.h>
#else 
  #define mVersion "M5StickCP 1.0"
  #include <M5StickCPlus.h>
#endif

// ポート定義
#define P_LED		10
#define P_BUZZER	2

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);

void _setLED(uint8_t onoff);
uint8_t _getSw(uint8_t button);
float getIMU(uint8_t index);
void _tone(uint8_t port, int16_t freq, int16_t ms);

#endif  // main_h
