#ifndef main_h
#define main_h

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);

#define CAMERA_ENABLED
//#define DEVICE_M5CAMERA
//#define DEVICE_M5TIMERCAM
//#define DEVICE_ESP32_CAM
//#define DEVICE_ESP32CAM
//#define DEVICE_UNITCAM
//#define DEVICE_UNITCAMS3
//#define QCAI
//#define DEVICE_CAMERATCH32
//#define CAMERA_MODEL_ESP32S3_EYE
//#define CAMERA_MODEL_SQUARE_CAM
//#define CAMERA_MODEL_XIAO32S3SENSE
//#define CAMERA_MODEL_CORES3
#define CAMERA_MODEL_ATOMS3R
#define XCLK_FREQ 10000000
#endif  // main_h
