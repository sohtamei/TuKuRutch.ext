#ifndef main_h
#define main_h

#define LGFX_AUTODETECT
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>

#include <TukurutchEsp.h>
#include <Preferences.h>

extern WebsocketsServer wsServer;
extern Preferences preferencesLCD;

void _setup(const char* ver);
void _loop(void);

#define NVSCONFIG_MAX  12

void _setupLCD(int lcdType, uint8_t *config_buf, int config_size);
int _getLcdConfig(uint8_t* buf);

extern lgfx::LGFX_Device *lcd;

#define GetL16(a) (((a)[1]<<8) | (a)[0])
#define SetL16(a, b) ((a)[1] = (b)>>8,  (a)[0] = ((b) & 0xFF))

#endif  // main_h
