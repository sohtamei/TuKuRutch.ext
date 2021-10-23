/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include "QuadCrawlerAI.h"
#include <analogRemote.h>
#include "TukurutchEspCamera2.h"
#include <TukurutchEsp.h>
#include "main.h"

static analogRemote remote(MODE_XYKEYS, /*port*/P_IRRX, /*funcLed*/NULL);

// QCAI
#define PWDN_GPIO_NUM	-1
#define RESET_GPIO_NUM	-1
#define XCLK_GPIO_NUM	32
#define SIOD_GPIO_NUM	26
#define SIOC_GPIO_NUM	27

#define Y9_GPIO_NUM		35
#define Y8_GPIO_NUM		34
#define Y7_GPIO_NUM		39
#define Y6_GPIO_NUM		36
#define Y5_GPIO_NUM		21
#define Y4_GPIO_NUM		19
#define Y3_GPIO_NUM		18
#define Y2_GPIO_NUM		5
#define VSYNC_GPIO_NUM	25
#define HREF_GPIO_NUM	23
#define PCLK_GPIO_NUM	22

WebsocketsServer wsServer;

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

void onConnect(String ip)
{
	quadCrawler_LED(true);
	wsServer.listen(PORT_WEBSOCKET);
	startCameraServer();
	Serial.println(ip);
}

void _camera_init(void)
{
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
	{
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

void _setup(const char* ver)
{
	Serial.begin(115200);
#ifdef LIDAR_ENABLED
	Serial2.begin(230400);
#endif
	quadCrawler_init();
	_camera_init();
	quadCrawler_colorWipe(COLOR_PURPLE);
	quadCrawler_tone(T_C4, 300);
	quadCrawler_tone(T_D4, 300);
	quadCrawler_tone(T_E4, 300);

	initWifi(ver, false, onConnect);
}

static uint8_t lastkey = 0;
static uint8_t originAdj = 0;
static uint8_t originAdjId;
static uint32_t sonner_time = 0;

static uint8_t lastSw4 = 1;
static int detect_sw4(void)
{
	int detect = 0;
	uint8_t sw4 = quadCrawler_SW();
	if(lastSw4!=sw4 && sw4==0)
		detect = 1;
	lastSw4 = sw4;
	return detect;
}

void loop_originAdj();
void _loop()
{
	if(originAdj) {
		loop_originAdj();
		return;
	}

	// リモコン処理：標準リモコン、アナログリモコンを受信し、キーコードに従ってロボットを動かす。
	#define A_DOWN_OFFSET  0x10
	remote.checkUpdated();            // リモコンコード受信
	uint8_t key = remote.keys;        // 受信したリモコンコード取得
	switch(key) {
	case BUTTON_A_XY:                 // アナログリモコン JOYSTICK操作のとき
		key = remote.xyKeys;
		break;
	case BUTTON_A_DOWN:
		key = remote.xyKeys + A_DOWN_OFFSET;  // アナログリモコン D ボタン押し
		delay(20);
		break;
	}

	if(key != lastkey) {
		lastkey = key;
		switch(key) {
		case BUTTON_POWER:
		case BUTTON_C:
		case BUTTON_B:
		case BUTTON_CENTER:
			break;
		case BUTTON_MENU:             // MENUボタン : ブザー3秒
			quadCrawler_tone(T_C5, 3000);
			break;
		case BUTTON_0:                // 0ボタン : LEDレインボー
			quadCrawler_tone(T_C5, 100);	//
			for (int n = 0; n < 5; n++) {
				quadCrawler_rainbow(5);
			}
			break;

		case XY_UP:                   // 上ボタン : 前進
		case BUTTON_UP:
			quadCrawler_colorWipe(COLOR_BLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_FW);
			break;
		case XY_LEFT:
		case BUTTON_LEFT:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_LEFT);
			break;
		case XY_RIGHT:
		case BUTTON_RIGHT:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_RIGHT);
			break;
		case XY_DOWN:
		case BUTTON_DOWN:
			quadCrawler_colorWipe(COLOR_RED);
			quadCrawler_Walk(quadCrawler_fast, COM_RW);
			break;
		case XY_UP_R:
		case XY_DOWN_L:
		case BUTTON_RETURN:
			quadCrawler_colorWipe(COLOR_GREEN);
			quadCrawler_Walk(quadCrawler_fast, COM_CW);
			break;
		case XY_UP_L:
		case XY_DOWN_R:
		case BUTTON_TEST:
			quadCrawler_colorWipe(COLOR_GREEN);
			quadCrawler_Walk(quadCrawler_fast, COM_CCW);
			break;

		case XY_UP_R + A_DOWN_OFFSET:
		case BUTTON_1:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_ALL_UP);
			break;
		case BUTTON_A_CENTER:
		case BUTTON_2:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_ALL_DOWN);
			break;
		case XY_DOWN + A_DOWN_OFFSET:
		case BUTTON_3:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_T_DOWN);
			break;
		case XY_UP + A_DOWN_OFFSET:
		case BUTTON_4:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_H_DOWN);
			break;
		case XY_LEFT + A_DOWN_OFFSET:
		case BUTTON_5:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_L_DOWN);
			break;
		case XY_RIGHT + A_DOWN_OFFSET:
		case BUTTON_6:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_R_DOWN);
			break;

		case BUTTON_A_UP:
		case BUTTON_7:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_T_UPDOWN);
			break;
		case BUTTON_A_LEFT:
		case BUTTON_8:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_L_R_UP);
			break;
		case BUTTON_A_RIGHT:
		case BUTTON_9:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_ALL_UPDOWN);
			break;
		default:                      // ボタンを離したとき : 停止
			quadCrawler_colorWipe(COLOR_PURPLE);
			quadCrawler_Walk(quadCrawler_fast, COM_STOP);
			break;
		}
	}

	// 歩行などのモーション処理、経過時間に応じてサーボモーターを制御する。
	if(remote.xyLevel >= 10) {
		// アナログリモコンのJOYSTICK操作のとき速度設定
		quadCrawler_setSpeed(25000 / remote.xyLevel, remote.x, remote.y);
	}
	quadCrawler_servoLoop();

	uint16_t elapsed = (millis() - sonner_time);
	if(elapsed >= 100) {
		sonner_time = millis();
		// SW4を押したとき初期姿勢にする (組み立て用)
		if(detect_sw4()) {
			int i;
			quadCrawler_colorWipe(COLOR_RED);
			for(i = 0; i < 4; i++)
				quadCrawler_setPose1(i, 0, 0);
			originAdj = 1;
			originAdjId = 0;
			return;
		}
/*
		// 超音波センサで障害物を検出したときブザーを鳴らす (100ms周期)
		double sonner_val = quadCrawler_getSonner();
		if (sonner_val < 8){
			quadCrawler_tone(T_C5, sonner_val * 10);

			if(lastkey == XY_UP || lastkey == BUTTON_UP) {
				quadCrawler_colorWipe(COLOR_PURPLE);
				quadCrawler_Walk(quadCrawler_fast, COM_STOP);
			}
		}
*/
	}
}


void loop_originAdj()
{
	uint16_t elapsed = (millis() - sonner_time);
	if(elapsed >= 100) {
		sonner_time = millis();
		if(detect_sw4()) {
			quadCrawler_colorWipe(COLOR_PURPLE);
			quadCrawler_Walk(quadCrawler_fast, COM_STOP);
			originAdj = 0;
			return;
		} else if(originAdj && !quadCrawler_checkServoON()) {
			quadCrawler_colorWipe(COLOR_PURPLE);
			quadCrawler_tone(T_C5, 500);
			originAdj = 0;
			return;
		}
	}

	remote.checkUpdated();            // リモコンコード受信
	uint8_t key = remote.keys;        // 受信したリモコンコード取得
	if(key != lastkey) {
		lastkey = key;
		int ret;
		switch(key) {
		case BUTTON_1: quadCrawler_colorRed(0); originAdjId = 1; break;
		case BUTTON_2: quadCrawler_colorRed(3); originAdjId = 3; break;
		case BUTTON_3: quadCrawler_colorRed(1); originAdjId = 2; break;
		case BUTTON_4: quadCrawler_colorRed(2); originAdjId = 4; break;
		}
		if(!originAdjId) return;

		switch(key) {
		case BUTTON_UP:
			ret = _calibServo((originAdjId-1)*2+0, CALIB_DEC);
			quadCrawler_tone(T_C5, (ret <= -30) ? 1000:50);
			break;

		case BUTTON_DOWN:
			ret = _calibServo((originAdjId-1)*2+0, CALIB_INC);
			quadCrawler_tone(T_C5, (ret >= 30) ? 1000:50);
			break;

		case BUTTON_LEFT:
			ret = _calibServo((originAdjId-1)*2+1, CALIB_DEC);
			quadCrawler_tone(T_C5, (ret <= -30) ? 1000:50);
			break;

		case BUTTON_RIGHT:
			ret = _calibServo((originAdjId-1)*2+1, CALIB_INC);
			quadCrawler_tone(T_C5, (ret >= 30) ? 1000:50);
			break;

		case BUTTON_CENTER:
			_calibServo((originAdjId-1)*2+0, CALIB_RESET);
			_calibServo((originAdjId-1)*2+1, CALIB_RESET);
			quadCrawler_tone(T_C5, 1000);
			break;
		}
	}
}
