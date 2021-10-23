#ifndef main_h
#define main_h

// ポート定義
#define P_BUZZER	12

void _setup(const char* ver);
void _loop(void);

uint16_t _getAdc1(uint8_t idx, uint16_t count);
void _setLED(uint8_t idx, uint8_t onoff);
uint8_t _getSw(uint8_t idx);
void _tone(uint8_t port, int16_t freq, int16_t ms);

#endif  // main_h
