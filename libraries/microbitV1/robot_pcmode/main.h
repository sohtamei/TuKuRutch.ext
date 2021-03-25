#ifndef main_h
#define main_h

#define mVersion "microbit 1.0"

void _setup(const char* ver);
void _loop(void);

void _displayText(char* text);
void _displayLed(uint32_t bitmap);
void _getSendData(void);
int _getTilt(uint8_t xy);

void sendBin(uint8_t* buf, uint8_t num);

#endif  // main_h
