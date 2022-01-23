#ifndef main_h
#define main_h

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);

//void _setLED(uint8_t onoff);
void espcamera_setCameraMode(uint8_t mode, uint8_t gain);

#define CAMERA_ENABLED
#define XCLK_FREQ 8000000
#endif  // main_h
