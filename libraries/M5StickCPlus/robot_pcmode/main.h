#ifndef main_h
#define main_h

//#define _M5StickC
#define _M5StickCPlus

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
void _setServo(uint8_t idx, int16_t data/*normal:0~180, continuous:-100~+100*/, uint8_t continuous);
void _setCar(uint8_t direction, uint8_t speed);
void setRoverC(int16_t F_L, int16_t F_R, int16_t R_L, int16_t R_R);
void setRoverC_XYR(int16_t x, int16_t y, int16_t role);
void moveRoverC(uint8_t dir, uint8_t speed);

#endif  // main_h
