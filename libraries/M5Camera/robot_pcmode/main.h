#ifndef main_h
#define main_h

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);

void _setCar(uint8_t direction, uint16_t speed, int16_t calib, int16_t duration);
void _setMotor(int16_t left, int16_t right/* -4 ~ +4 */,
	int16_t calib,   /* -50(Left) ~ +50(right) */
	int16_t duration);
void _setServo(uint8_t idx, int16_t data/*0~180*/);
void _setPwm(uint8_t idx, int16_t data);
void _stopServo(void);
void _setLED(uint8_t onoff);
char* _downloadCal(int16_t id, char* base64);
char* _getCal(void);

#define DEVICE_M5CAMERA
//#define DEVICE_M5TIMERCAM
//#define DEVICE_ESP32_CAM
//#define DEVICE_ESP32CAM
//#define QC
#endif  // main_h
