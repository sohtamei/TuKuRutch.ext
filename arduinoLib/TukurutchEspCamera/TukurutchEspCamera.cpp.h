/* copyright (C) 2020 Sohta. */

void espcamera_setLED(uint8_t onoff)
{
#if P_LED != -1
#if P_LED_NEG
	digitalWrite(P_LED, !onoff);
#else
	digitalWrite(P_LED, onoff);
#endif
	pinMode(P_LED, OUTPUT);
#endif
}

void espcamera_setCameraMode(uint8_t mode, uint8_t gain)
{
	sensor_t * s = esp_camera_sensor_get();
	uint8_t enabled = 1;
	switch(mode) {
	case 1:		// color detect
		s->set_agc_gain(s, gain);
		enabled = 0;
	case 0:		// normal
		s->set_whitebal(s, enabled);
		if(s->id.PID == OV2640_PID)
			s->set_awb_gain(s, enabled);
		s->set_gain_ctrl(s, enabled);
		break;
	}
	return;
}

static uint8_t connected = false;
void espcamera_onConnect()
{
	espcamera_setLED(1);
	startCameraServer();
	connected = true;
}

void espcamera_setup(void)
{
	espcamera_setLED(0);

	camera_config_t config;
	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer = LEDC_TIMER_0;
	config.pin_d0 = Y2_GPIO_NUM;
	config.pin_d1 = Y3_GPIO_NUM;
	config.pin_d2 = Y4_GPIO_NUM;
	config.pin_d3 = Y5_GPIO_NUM;
	config.pin_d4 = Y6_GPIO_NUM;
	config.pin_d5 = Y7_GPIO_NUM;
	config.pin_d6 = Y8_GPIO_NUM;
	config.pin_d7 = Y9_GPIO_NUM;
	config.pin_xclk = XCLK_GPIO_NUM;
	config.pin_pclk = PCLK_GPIO_NUM;
	config.pin_vsync = VSYNC_GPIO_NUM;
	config.pin_href = HREF_GPIO_NUM;
	config.pin_sscb_sda = SIOD_GPIO_NUM;
	config.pin_sscb_scl = SIOC_GPIO_NUM;
	config.pin_pwdn = PWDN_GPIO_NUM;
	config.pin_reset = RESET_GPIO_NUM;
	config.xclk_freq_hz = XCLK_FREQ;
	config.pixel_format = PIXFORMAT_JPEG;
	//init with high specs to pre-allocate larger buffers
	if(psramFound()){
		config.frame_size = FRAMESIZE_UXGA;
		config.jpeg_quality = 10;
		config.fb_count = 2;
	} else {
		config.frame_size = FRAMESIZE_SVGA;
		config.jpeg_quality = 12;
		config.fb_count = 1;
	}

	// camera init
	esp_err_t err = esp_camera_init(&config);
	if (err != ESP_OK) {
		Serial.printf("Camera init failed with error 0x%x\n", err);
		return;
	}

	sensor_t * s = esp_camera_sensor_get();
	if (s->id.PID == OV3660_PID) {
		s->set_vflip(s, 1);			//flip it back
		s->set_brightness(s, 1);	//up the blightness just a bit
		s->set_saturation(s, -2);	//lower the saturation
		s->set_vflip(s, 0);
		s->set_hmirror(s, 0);
	} else {
		s->set_hmirror(s, 1);
	}
	s->set_framesize(s, FRAMESIZE_HVGA);
}

static uint8_t lastStream_state = -1;
static uint32_t lastStream;
void espcamera_loop(void)
{
	if(lastStream_state != stream_state) {
		lastStream_state = stream_state;
		lastStream = millis();
		espcamera_setLED(connected);
#if defined(DEVICE_M5TIMERCAM)
		digitalWrite(BAT_OUTPUT_HOLD_PIN,1);
#endif
	} else {
		int32_t elapsed = ((millis() - lastStream) & 0x7FFFFFFF) / 1000;
		if(stream_state) {
			espcamera_setLED(elapsed & 1);
		} else {
#if defined(DEVICE_M5TIMERCAM)
			if(elapsed > 120) {
				Serial.println("disconnect battery");
				digitalWrite(BAT_OUTPUT_HOLD_PIN,0);
				lastStream = millis();
			}
#endif
		}
	}
}

#include "sccb.cpp.h"
