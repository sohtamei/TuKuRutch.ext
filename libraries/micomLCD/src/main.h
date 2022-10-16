#ifndef main_h
#define main_h

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <TukurutchEsp.h>
#include <Preferences.h>

extern WebsocketsServer wsServer;
extern Preferences preferencesLCD;

void _setup(const char* ver);
void _loop(void);

#define NVSCONFIG_MAX  12

void _setupLCD(int lcdType, uint8_t *config_buf, int config_size);

enum {
	LCDTYPE_DEFAULT = 0,
	LCDTYPE_SSD1306 = 1,
	LCDTYPE_SSD1306_32 = 2,
	LCDTYPE_SSD1331 = 3,
	LCDTYPE_3248S035 = 4,
	LCDTYPE_GC9A01 = 5,
};

extern lgfx::LGFX_Device *lcd;

#endif  // main_h
