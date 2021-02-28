/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <libb64/cencode.h>
#include <libb64/cdecode.h>
#include <Preferences.h>
#include <TukurutchEspCamera.h>
#include <TukurutchEsp.h>
#include "main.h"

// ポート定義

#if defined(DEVICE_M5TIMERCAM)
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
  #define P_SRV0			13
  #define P_SRV1			4

  #include "bmm8563.h"
  #define BAT_OUTPUT_HOLD_PIN	33	// 0-bat power disable
  #define BAT_ADC_PIN			38

#elif defined(DEVICE_M5CAMERA)
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

#elif defined(DEVICE_ESP32_CAM)
// ESP32-CAM
  #define PWDN_GPIO_NUM     32
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
  #define P_SRV0			13
  #define P_SRV1			4
  // flash-4
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
  #define P_SRV0			4
  #define P_SRV1			13

#else
  #error
#endif

WebsocketsServer wsServer;
static Preferences preferencesRobot;

#define numof(a) (sizeof(a)/sizeof((a)[0]))


enum {
	CAL0P = 0,
	CAL0M,
	CAL1P,
	CAL1M,
};

enum {
	PWM0,
	PWM25,
	PWM50,
	PWM75,
	PWM100,
};

struct calibrate {
	uint16_t id;
	uint16_t ver;
	uint16_t T;
	uint16_t _reserve;
	uint16_t pwm[4][4];		// CAL0P,CAL0M,CAL1P,CAL1M  PWM0,PWM25,PWM50,PWM75
	 int16_t maxOffset[2];
	 int16_t speedMax[4];		// CAL0P,CAL0M,CAL1P,CAL1M
} static cal;
static uint16_t pwm0[2];

static const struct calibrate calInit = {
	0,
	1,
	18,
	0,
	{{308,314,319,336},{297,292,287,270},{295,290,285,267},{309,315,320,339}},
	{0,0},
	{2000,-2000,2000,-2000},
};

enum {
	SERVO_IDLE,
	SERVO_STOP_REQ,
	SERVO_OFF_REQ,
	SERVO_REV_REQ,
};
static uint8_t servo_stt = 0;
static uint32_t servo_time = 0;

void _setLED(uint8_t onoff)
{
#if P_LED_NEG
	digitalWrite(P_LED, !onoff);
#else
	digitalWrite(P_LED, onoff);
#endif
	pinMode(P_LED, OUTPUT);
}
/*
uint8_t _getSw(uint8_t idx)
{
	int ret = 0;
	pinMode(P_SRV1, INPUT_PULLDOWN);
	pinMode(P_SRV0, OUTPUT);

	digitalWrite(P_SRV0, 1); delay(5);
	if(digitalRead(P_SRV1) == 1) {
		ret = 1;
	}
	pinMode(P_SRV0, INPUT);
	return ret;
}
*/
static uint16_t pwm_last[2];
static uint16_t pwm_req[2];
static uint16_t pwm_duration = 0;

#define PWM_MIN     143
#define PWM_NEUTRAL 307
#define PWM_MAX     471
const struct {uint8_t ledc; uint8_t port;} servoTable[] = {{8,P_SRV0},{9,P_SRV1}};
void _setPwm(uint8_t idx, int16_t data)
{
	ledcAttachPin(servoTable[idx].port, servoTable[idx].ledc);
	ledcWrite(servoTable[idx].ledc, data);
	pwm_last[idx] = data;
}

static const uint16_t maxOffsetTbl[] = {164,83,70,63,58,54,51,48,45,43,41,40,38,37,35,34,33,32,31,30,29,28,27,26,26,25,24,24,23,22,22,21,21,20,19,19,18,18,17,17,16,16,16,15,15,14,14,14,13,13,12};

int16_t limitOffset(int16_t offset)
{
		 if(offset >= (int) numof(maxOffsetTbl)) offset =   numof(maxOffsetTbl)-1;
	else if(offset <= (int)-numof(maxOffsetTbl)) offset = -(numof(maxOffsetTbl)-1);
	return offset;
}

void _setMotor(int16_t left, int16_t right/* -4 ~ +4 */,
	int16_t calib,   /* -50(Left) ~ +50(right) */
	int16_t duration)
{
		 if(left >  PWM100) left =  PWM100;
	else if(left < -PWM100) left = -PWM100;

		 if(right >  PWM100) right =  PWM100;
	else if(right < -PWM100) right = -PWM100;

	uint16_t pwmL = 0;
	uint16_t pwmR = 0;
	int data = 0;
	int calib4 = (calib + ((calib>=0)?2:-2) )/4;
	switch(left) {
	case PWM100:
		data = limitOffset(cal.maxOffset[0] - calib*2);
		if(data >= 0)
			pwmL = cal.pwm[CAL0P][PWM0] + maxOffsetTbl[data];
		else
			pwmL = PWM_MAX;
		break;

	case -PWM100:
		data = limitOffset(cal.maxOffset[1] - calib*2);
		if(data >= 0)
			pwmL = cal.pwm[CAL0M][PWM0] - maxOffsetTbl[data];
		else
			pwmL = PWM_MIN;
		break;

	case PWM0:
		if(calib == 0)
			pwmL = 0;
		else if(calib > 0)
			pwmL = cal.pwm[CAL0P][PWM0] + calib4;
		else
			pwmL = cal.pwm[CAL0M][PWM0] + calib4;
		break;

	default:
		if(left >= 0)
			pwmL = cal.pwm[CAL0P][ left] + calib4;
		else
			pwmL = cal.pwm[CAL0M][-left] + calib4;
		break;
	}

	switch(right) {
	case PWM100:
		data = limitOffset(cal.maxOffset[0] - calib*2);
		if(data >= 0)
			pwmR = PWM_MIN;
		else
			pwmR = cal.pwm[CAL1P][PWM0] - maxOffsetTbl[-data];
		break;

	case -PWM100:
		data = limitOffset(cal.maxOffset[1] - calib*2);
		if(data >= 0)
			pwmR = PWM_MAX;
		else
			pwmR = cal.pwm[CAL1M][PWM0] + maxOffsetTbl[-data];
		break;

	case PWM0:
		if(calib == 0)
			pwmR = 0;
		else if(calib > 0)
			pwmR = cal.pwm[CAL1M][PWM0] + calib4;
		else
			pwmR = cal.pwm[CAL1P][PWM0] + calib4;
		break;

	default:
		if(right >= 0)
			pwmR = cal.pwm[CAL1P][ right] + calib4;
		else
			pwmR = cal.pwm[CAL1M][-right] + calib4;
		break;
	}
//Serial.printf("%d %d\n", pwmL, pwmR);

	servo_stt = SERVO_IDLE;
	if(pwm_last[0] && pwmL && ((pwm_last[0]>=pwm0[0]) != (pwmL>=pwm0[0]))) {
		_setPwm(0, 0);
		servo_stt = SERVO_REV_REQ;
	} else {
		_setPwm(0, pwmL);
	}

	if(pwm_last[1] && pwmR && ((pwm_last[1]>=pwm0[1]) != (pwmR>=pwm0[1]))) {
		_setPwm(1, 0);
		servo_stt = SERVO_REV_REQ;
	} else {
		_setPwm(1, pwmR);
	}

	if(servo_stt == SERVO_REV_REQ) {
		servo_time = millis() + 40;
		pwm_req[0] = pwmL;
		pwm_req[1] = pwmR;
		pwm_duration = duration;

	} else if(duration) {
		servo_stt = SERVO_STOP_REQ;
		servo_time = millis() + duration;
	} else {
		servo_stt = SERVO_IDLE;
		servo_time = 0;
	}
}

void _stopServo(void)
{
	servo_stt = SERVO_IDLE;
	servo_time = 0;
	_setPwm(0, 0);
	_setPwm(1, 0);
}

enum {
	CMD_STOP = 0,
	CMD_FORWARD,
	CMD_LEFT,
	CMD_RIGHT,
	CMD_BACK,
	CMD_ROLL_LEFT,
	CMD_ROLL_RIGHT,
	CMD_CALIBRATION,
};

struct { int16_t L; int16_t R;} static const dir_table[7] = {
//  L   R
	{ 0,  0},  // 0:STOP
	{ 1,  1},  // 1:FORWARD
	{ 0,  1},  // 2:LEFT
	{ 1,  0},  // 3:RIGHT
	{-1, -1},  // 4:BACK
	{-1,  1},  // 5:ROLL_LEFT
	{ 1, -1},  // 6:ROLL_RIGHT
					   // 7:CALIBRATION
};

void _setCar(uint8_t direction, uint16_t speed, int16_t calib, int16_t duration)
{
	if(direction >= CMD_CALIBRATION) {
		_setPwm(0, PWM_NEUTRAL);
		_setPwm(1, PWM_NEUTRAL);
	} else {
		_setMotor(speed*dir_table[direction].L, speed*dir_table[direction].R, calib, duration);
	}
}

/////////////////////////////////////////////

char* _downloadCal(int16_t id, char* base64)
{
	if(base64 && base64[0]) {
		int len = base64_decode_chars(base64, strlen(base64), strBuf);
		if(len == sizeof(cal)) {
			memcpy((char*)&cal, strBuf, len);
			preferencesRobot.putBytes("calib", &cal, sizeof(cal));
			pwm0[0] = (cal.pwm[CAL0P][PWM0] + cal.pwm[CAL0M][PWM0])/2;
			pwm0[1] = (cal.pwm[CAL1P][PWM0] + cal.pwm[CAL1M][PWM0])/2;
			snprintf(strBuf, sizeof(strBuf), "OK");
		} else {
			snprintf(strBuf, sizeof(strBuf), "Error");
		}
		return strBuf;
	}

	char url[50];
	if(id==0) id=cal.id;
	snprintf(url, sizeof(url), "http://sohta02.web.fc2.com/calib/M5CC%04d.txt", id);
	memset(strBuf, 0, sizeof(strBuf));
	HTTPClient http;
	http.begin(url);
	int httpCode = http.GET();
	if(httpCode == HTTP_CODE_OK) {
		String payload = http.getString();
		Serial.println(payload);
		if(base64_decode_expected_len(payload.length()) >= sizeof(strBuf)) goto Error;

		int len = base64_decode_chars((const char*)payload.c_str(), payload.length(), strBuf);
		if(len != sizeof(cal)) goto Error;
		memcpy((char*)&cal, strBuf, len);
		preferencesRobot.putBytes("calib", &cal, sizeof(cal));
		pwm0[0] = (cal.pwm[CAL0P][PWM0] + cal.pwm[CAL0M][PWM0])/2;
		pwm0[1] = (cal.pwm[CAL1P][PWM0] + cal.pwm[CAL1M][PWM0])/2;

		uint16_t i;
		for(i=0; i<len; i++) Serial.printf("%02x", ((uint8_t*)&cal)[i]);
		Serial.println();
		snprintf(strBuf, sizeof(strBuf), "OK:%s", payload.c_str());
	} else {
		Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
Error:
		snprintf(strBuf, sizeof(strBuf), "Error");
	}
	http.end();
	return strBuf;
}

char* _getCal(void)
{
	base64_encode_chars((char*)&cal, sizeof(cal), strBuf);
	return strBuf;
}

void _setServo(uint8_t idx, int16_t data/*0~180*/)
{
	if(idx >= numof(servoTable)) return;

	#define srvMin 103		// 0.5ms/20ms*4096 = 102.4 (-90c)
	#define srvMax 491		// 2.4ms/20ms*4096 = 491.5 (+90c)
	if(data < 0) data = 0;
	else if(data > 180) data = 180;

	uint16_t pwmWidth = (data * (srvMax - srvMin)) / 180 + srvMin;
	_setPwm(idx, pwmWidth);
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

void M5CameraCar_init(void)
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
	config.xclk_freq_hz = 20000000;
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

void _setup(const char* ver)
{
	_setLED(0);

	Serial.begin(115200);

#if defined(DEVICE_M5TIMERCAM)
	pinMode(BAT_OUTPUT_HOLD_PIN,OUTPUT);
	digitalWrite(BAT_OUTPUT_HOLD_PIN,1);
	bmm8563_init();
#endif
	M5CameraCar_init();
/*
	if(_getSw(0)) {
		_setLED(1);
		delay(100);
		_setLED(0);
		Serial.println("Waiting for SmartConfig.");
		WiFi.mode(WIFI_STA);
		WiFi.beginSmartConfig();
		while (!WiFi.smartConfigDone()) {
			delay(1000);
			_setLED(1);
			delay(100);
			_setLED(0);
		}
		Serial.println("SmartConfig received.");
	}
*/
	// ServoCar
	ledcSetup(servoTable[0].ledc, 50/*Hz*/, 12/*bit*/);
	ledcSetup(servoTable[1].ledc, 50/*Hz*/, 12/*bit*/);

	initWifi(ver, false, onConnect);

	preferencesRobot.begin("M5CameraCar", false);
	if(preferencesRobot.getBytes("calib", &cal, sizeof(cal)) < sizeof(cal)) {
		Serial.println("init calib");
		memset(&cal, 0, sizeof(cal));
		preferencesRobot.putBytes("calib", &cal, sizeof(cal));
	}
	pwm0[0] = (cal.pwm[CAL0P][PWM0] + cal.pwm[CAL0M][PWM0])/2;
	pwm0[1] = (cal.pwm[CAL1P][PWM0] + cal.pwm[CAL1M][PWM0])/2;
}

static uint8_t lastStream_state = -1;
static uint32_t lastStream;
void _loop(void)
{
	if(servo_stt) {
		int32_t d = servo_time - millis();
		if(d <= 0) {
			switch(servo_stt) {
			case SERVO_REV_REQ:
				if(pwm_duration) {
					servo_stt = SERVO_STOP_REQ;
					servo_time = millis() + pwm_duration;
				} else {
					servo_stt = SERVO_IDLE;
					servo_time = 0;
				}
				_setPwm(0, pwm_req[0]);
				_setPwm(1, pwm_req[1]);
				break;

			case SERVO_STOP_REQ:
			case SERVO_OFF_REQ:
			default:
				servo_stt = SERVO_IDLE;
				servo_time = 0;
				_setPwm(0, 0);
				_setPwm(1, 0);
				break;
			}
		}
	}

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
