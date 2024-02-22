#ifndef main_h
#define main_h

#include <SD.h>
#include <M5Unified.h>

#include <TukurutchEsp.h>
extern WebsocketsServer wsServer;

void _setup(const char* ver);
void _loop(void);

int _getConfig(uint8_t* buf);

uint8_t _getSw(uint8_t button);
float getIMU(uint8_t index);
int getIMUs(uint8_t index, uint8_t* buf);
void __tone(int16_t freq, int16_t ms);
char* _getFilelist(void);
int _removeFiles(void);
void _drawFile(const char* filename, int x, int y);
void _setAutomode(const char* filename, int duration);
extern uint8_t comMode;

#define GetL16(a) (((a)[1]<<8) | (a)[0])
#define SetL16(a, b) ((a)[1] = (b)>>8,  (a)[0] = ((b) & 0xFF))

#endif  // main_h
