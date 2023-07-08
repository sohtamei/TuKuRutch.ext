#ifndef main_h
#define main_h

#define LGFX_AUTODETECT
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>

#include <TukurutchEsp.h>
extern WebsocketsServer wsServer;

void _setup(const char* ver);
void _loop(void);

void _setupLCD(int lcdType, uint8_t *config_buf, int config_size);
int _getLcdConfig(uint8_t* buf);
void _setLcdConfig(int lcdType, uint8_t *config_buf, int config_size);

extern lgfx::LGFX_Device *lcd;

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
  #define CAMERA_MODEL_SQUARE_CAM
#define XCLK_FREQ 8000000
#endif  // main_h
