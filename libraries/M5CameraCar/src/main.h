#ifndef main_h
#define main_h

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);

void _setLED(uint8_t onoff);
void _setCameraMode(uint8_t mode, uint8_t gain);

void _setCar(uint8_t direction, uint16_t speed, int16_t calib, int16_t duration);
void _setMotor(int16_t left, int16_t right/* -4 ~ +4 */,
	int16_t calib,   /* -50(Left) ~ +50(right) */
	int16_t duration);
void _setCar2(uint8_t direction, int16_t speed, int16_t duration);
void _stopServo(void);
char* _downloadCal(int16_t id, char* base64);
char* _getCal(void);

#define CAMERA_ENABLED
#define DEVICE_M5CAMERA
//#define DEVICE_M5TIMERCAM
//#define DEVICE_ESP32_CAM
//#define DEVICE_ESP32CAM
//#define DEVICE_UNITCAM
//#define QCAI
//#define DEVICE_CAMERATCH32
#define XCLK_FREQ 10000000
#endif  // main_h
