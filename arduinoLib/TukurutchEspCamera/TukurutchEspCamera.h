#include "esp_camera.h"
#include "esp_camera_port.h"

void startCameraServer(void);
extern uint8_t stream_state;	// (1) - streaming中

void espcamera_setLED(uint8_t onoff);
void espcamera_setCameraMode(uint8_t mode, uint8_t gain);
void espcamera_onConnect();
void espcamera_setup(void);
void espcamera_loop(void);
