#ifndef main_h
#define main_h

#include <U8x8lib.h>
#include <DHT.h>
#include <Seeed_BMP280.h>

extern U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8;
extern DHT* dht;
extern BMP280 bmp280;


// ポート定義

#define P_LED1		13
#define P_LED2		4
#define P_BUZZER	5
#define P_BUTTON	6
#define P_TEMP_HUM	3

#define A_VOLUME	0
#define A_LIGHT		6
#define A_SOUND		2

void _setup(const char* ver);
void _loop(void);

float _getAccel(uint8_t xyz);

// robot_pcmode.ino
void _tone(uint8_t port, int16_t freq, int16_t ms);

#endif  // main_h
