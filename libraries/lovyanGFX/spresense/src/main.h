#ifndef main_h
#define main_h

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx_user/LGFX_SPRESENSE_sample.hpp>

#if defined(ESP32)
  #include <TukurutchEsp.h>
  extern WebsocketsServer wsServer;
#else
  #include <stdio.h>
#endif

void _setup(const char* ver);
void _loop(void);

void _setupLCD(int lcdType, uint8_t *config_buf, int config_size);
int _getLcdConfig(uint8_t* buf);
void _setLcdConfig(int lcdType, uint8_t *config_buf, int config_size);

extern lgfx::LGFX_Device *lcd;

#define GetL16(a) (((a)[1]<<8) | (a)[0])
#define SetL16(a, b) ((a)[1] = (b)>>8,  (a)[0] = ((b) & 0xFF))

#endif  // main_h
