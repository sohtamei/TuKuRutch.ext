#ifndef main_h
#define main_h

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);

void _setLED(uint8_t onoff);
uint8_t _getSw(uint8_t button);
float getIMU(uint8_t index);
void _tone(int sound, int ms);
void _setServo(uint8_t idx, int16_t data/*normal:0~180, continuous:-100~+100*/, uint8_t continuous);
void _setCar(uint8_t direction, uint8_t speed);

#endif  // main_h
