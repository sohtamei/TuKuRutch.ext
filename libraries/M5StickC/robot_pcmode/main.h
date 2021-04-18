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

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);

void _setLED(uint8_t onoff);
uint8_t _getSw(uint8_t button);
float getIMU(uint8_t index);
void _tone(int sound, int ms);
void _beep(void);

#endif  // main_h
