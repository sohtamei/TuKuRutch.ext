/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include "TukurutchEspCamera3.h"
#include "TukurutchEsp.h"
#include "main.h"

// ポート定義

#if defined(DEVICE_M5CAMERA)
// M5camera modelB
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     22
  #define SIOC_GPIO_NUM     23

  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

  #define P_LED				14
  #define P_LED_NEG			1
  #define P_SRV0			13
  #define P_SRV1			4

#elif defined(DEVICE_M5TIMERCAM)
// M5TimerCam
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23

  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

  #define P_LED				2
  #define P_LED_NEG			0

  #include "bmm8563.h"
  #define BAT_OUTPUT_HOLD_PIN	33	// 0-bat power disable
  #define BAT_ADC_PIN			38

  #define INIT_IO() { \
	pinMode(BAT_OUTPUT_HOLD_PIN,OUTPUT); \
	digitalWrite(BAT_OUTPUT_HOLD_PIN,1); \
	bmm8563_init(); \
	bmm8563_close(); \
  }
#elif defined(DEVICE_ESP32_CAM)
// ESP32-CAM
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM     0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27

  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM       5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22

  #define P_LED				33
  #define P_LED_NEG			1

  #define INIT_IO() { \
	digitalWrite(32, 0); pinMode(32, OUTPUT); /*CAM PWR*/ \
	digitalWrite(4, 0); pinMode(4, OUTPUT); /*FLASH*/ \
  }
  // io-2,4,12,13,14,15

#elif defined(DEVICE_ESP32CAM)
// ESP32cam
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23

  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       17
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

  #define P_LED				16
  #define P_LED_NEG			0

#elif defined(DEVICE_UNITCAM)
// UnitCam
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23

  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

  #define P_LED				4
  #define P_LED_NEG			1

#elif defined(QCAI)
// QCAI
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM     32
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27

  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM       5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22

  #define P_LED				0
  #define P_LED_NEG			0

#elif defined(DEVICE_CAMERATCH32)
// CAMERATCH32
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM     32
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27

  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM       5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22

  #define P_LED				15
  #define P_LED_NEG			0

#elif defined(CAMERA_MODEL_ESP32S3_EYE)
// ESP32S3 camera Freenove
  #define PWDN_GPIO_NUM -1
  #define RESET_GPIO_NUM -1
  #define XCLK_GPIO_NUM 15
  #define SIOD_GPIO_NUM 4
  #define SIOC_GPIO_NUM 5

  #define Y2_GPIO_NUM 11
  #define Y3_GPIO_NUM 9
  #define Y4_GPIO_NUM 8
  #define Y5_GPIO_NUM 10
  #define Y6_GPIO_NUM 12
  #define Y7_GPIO_NUM 18
  #define Y8_GPIO_NUM 17
  #define Y9_GPIO_NUM 16

  #define VSYNC_GPIO_NUM 6
  #define HREF_GPIO_NUM 7
  #define PCLK_GPIO_NUM 13

  #define P_LED				2
  #define P_LED_NEG			0

#else
  #error
#endif

WebsocketsServer wsServer;

void _setLED(uint8_t onoff)
{
return;
#if P_LED_NEG
	digitalWrite(P_LED, !onoff);
#else
	digitalWrite(P_LED, onoff);
#endif
	pinMode(P_LED, OUTPUT);
}

void _setCameraMode(uint8_t mode, uint8_t gain)
{
	sensor_t * s = esp_camera_sensor_get();
	uint8_t enabled = 1;
	switch(mode) {
	case 1:		// color detect
		s->set_agc_gain(s, gain);
		enabled = 0;
	case 0:		// normal
		s->set_whitebal(s, enabled);
		s->set_awb_gain(s, enabled);
		s->set_gain_ctrl(s, enabled);
		break;
	}
	return;
}

static uint8_t connected = false;
void onConnect(String ip)
{
	_setLED(1);

	wsServer.listen(PORT_WEBSOCKET);
	startCameraServer();
	Serial.println(ip);
	connected = true;
}

extern "C" {
	esp_err_t camera_enable_out_clock(camera_config_t* config);
}
void ll_cam_set_pin(camera_config_t* config);

camera_config_t config;

void _setLedc(uint32_t clock)
{
	config.xclk_freq_hz = clock;
	camera_enable_out_clock(&config);
}

void _camera_init(void)
{
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
	config.pin_sccb_sda = SIOD_GPIO_NUM;
	config.pin_sccb_scl = SIOC_GPIO_NUM;
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
	ll_cam_set_pin(&config);
	camera_enable_out_clock(&config);
//	PIN_SET_DRV(GPIO_PIN_MUX_REG[32],0);
//	Serial.printf("PIN_MUX[32]=%08x\n",*(volatile uint32_t*)(GPIO_PIN_MUX_REG[32]));
/*
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
*/
}

// ESP32S3
#include "soc/system_reg.h"
void ll_cam_set_pin(camera_config_t* config)
{
    // ll_cam_set_pin
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[config->pin_xclk], PIN_FUNC_GPIO);
    gpio_set_direction((gpio_num_t)config->pin_xclk, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode((gpio_num_t)config->pin_xclk, GPIO_FLOATING);
    gpio_matrix_out((gpio_num_t)config->pin_xclk, CAM_CLK_IDX, false, false);

    // ll_cam_config
    if (REG_GET_BIT(SYSTEM_PERIP_CLK_EN1_REG, SYSTEM_LCD_CAM_CLK_EN) == 0) {
        REG_CLR_BIT(SYSTEM_PERIP_CLK_EN1_REG, SYSTEM_LCD_CAM_CLK_EN);
        REG_SET_BIT(SYSTEM_PERIP_CLK_EN1_REG, SYSTEM_LCD_CAM_CLK_EN);
        REG_SET_BIT(SYSTEM_PERIP_RST_EN1_REG, SYSTEM_LCD_CAM_RST);
        REG_CLR_BIT(SYSTEM_PERIP_RST_EN1_REG, SYSTEM_LCD_CAM_RST);
    }
}

/*
#include <driver/i2s.h>

#define I2S_NUM_AMP                     I2S_NUM_1
#define I2S_SAMPLE_RATE                 39062 // 39.062kHz x 256 = 10M
//#define I2S_SAMPLE_RATE                 78125 // 78.125kHz x 256 = 20M
//#define I2S_SAMPLE_RATE                 156250 // 156.25kHz x 256 = 40M
//#define I2S_SAMPLE_RATE                 312500 // 312.5kHz x 256 = 80M
  i2s_config_t i2s_config = {
    .mode                 = (i2s_mode_t)I2S_MODE_MASTER,
    .sample_rate          = I2S_SAMPLE_RATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT, //stereo
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 2,
    .dma_buf_len          = 512,
    .use_apll             = false,
    .tx_desc_auto_clear   = false,
    .fixed_mclk           = 0
  };
void setI2s(uint32_t sample_rate) {
  i2s_config.sample_rate = sample_rate;
  i2s_stop(I2S_NUM_AMP);
  i2s_driver_uninstall(I2S_NUM_AMP);
  i2s_driver_install(I2S_NUM_AMP, &i2s_config, 0, NULL);
  PIN_FUNC_SELECT(IO_MUX_GPIO0_REG, FUNC_GPIO0_CLK_OUT1);
}

void i2s_enable() {
  i2s_driver_install(I2S_NUM_AMP, &i2s_config, 0, NULL);
  PIN_FUNC_SELECT(IO_MUX_GPIO0_REG, FUNC_GPIO0_CLK_OUT1);
}

void i2s_disable() {
  i2s_stop(I2S_NUM_AMP);
  i2s_driver_uninstall(I2S_NUM_AMP);
}
*/
void _setup(const char* ver)
{
	_setLED(0);

	Serial.begin(115200);

#if defined(INIT_IO)
	INIT_IO();
#endif
	_camera_init();

//	i2s_enable();

	initWifi(ver, false, onConnect);
}

static uint8_t lastStream_state = -1;
static uint32_t lastStream;
void _loop(void)
{
	if(lastStream_state != stream_state) {
		lastStream_state = stream_state;
		lastStream = millis();
		_setLED(connected);
#if defined(DEVICE_M5TIMERCAM)
		digitalWrite(BAT_OUTPUT_HOLD_PIN,1);
#endif
	} else {
		int32_t elapsed = ((millis() - lastStream) & 0x7FFFFFFF) / 1000;
		if(stream_state) {
			_setLED(elapsed & 1);
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

	delay(40);
}
