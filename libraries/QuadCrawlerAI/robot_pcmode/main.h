#ifndef main_h
#define main_h

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);

//void _setLED(uint8_t onoff);
void _setCameraMode(uint8_t mode, uint8_t gain);

void _respLidar();
#endif  // main_h
