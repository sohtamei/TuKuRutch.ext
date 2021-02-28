#ifndef main_h
#define main_h

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);

uint16_t _getAdc1(uint8_t idx, uint16_t count);
void _setLED(uint8_t idx, uint8_t onoff);
uint8_t _getSw(uint8_t idx);
void _tone(int sound, int ms);

#endif  // main_h
