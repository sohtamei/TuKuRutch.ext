#ifndef main_h
#define main_h

#define M5STACK_MPU6886 
// #define M5STACK_MPU9250 
#include <M5Stack.h>

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);

void _setLED(uint8_t onoff);
uint8_t _getSw(uint8_t button);
float getIMU(uint8_t index);
void _tone(int sound, int ms);
void _beep(void);

#endif  // main_h
