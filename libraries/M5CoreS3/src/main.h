#ifndef main_h
#define main_h

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

#define GetL16(a) (((a)[1]<<8) | (a)[0])
#define SetL16(a, b) ((a)[1] = (b)>>8,  (a)[0] = ((b) & 0xFF))

#define CAMERA_ENABLED
//#define DEVICE_M5CAMERA
//#define DEVICE_M5TIMERCAM
//#define DEVICE_ESP32_CAM
//#define DEVICE_ESP32CAM
//#define DEVICE_UNITCAM
//#define QCAI
//#define DEVICE_CAMERATCH32
//#define CAMERA_MODEL_ESP32S3_EYE
//#define CAMERA_MODEL_SQUARE_CAM
  #define CAMERA_MODEL_CORES3
#define XCLK_FREQ 20000000
#define I2C_WIRE1
#endif  // main_h
