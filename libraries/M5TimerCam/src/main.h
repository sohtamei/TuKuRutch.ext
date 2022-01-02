#ifndef main_h
#define main_h

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);

#define CAMERA_ENABLED
//#define DEVICE_M5CAMERA
#define DEVICE_M5TIMERCAM
//#define DEVICE_ESP32_CAM
//#define DEVICE_ESP32CAM
//#define DEVICE_UNITCAM
//#define QCAI
//#define DEVICE_CAMERATCH32
#define XCLK_FREQ 10000000
#endif  // main_h
