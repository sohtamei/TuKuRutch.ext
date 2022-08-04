#include "esp_camera.h"
#include "camera_pins.h"

void startCameraServer(void);
extern bool isStreaming;	// (1) - streaming中

void espcamera_setLED(uint8_t onoff);
void espcamera_setCameraMode(uint8_t mode, uint8_t gain);
void espcamera_onConnect();
void espcamera_setup(void);
void espcamera_loop(void);
