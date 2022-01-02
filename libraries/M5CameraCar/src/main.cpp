/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>

#include <TukurutchEsp.h>
#include "main.h"
#include <TukurutchEspCamera.h>

#include <TukurutchEspCamera.cpp.h>

WebsocketsServer wsServer;

/////////////////////////////////////////////

#include <Wire.h>
#include <HTTPClient.h>
#include <libb64/cencode.h>
#include <libb64/cdecode.h>
#include <Preferences.h>

#define numof(a) (sizeof(a)/sizeof((a)[0]))

#define P_SRV0			13
#define P_SRV1			4

static Preferences preferencesRobot;

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
	SERVO_REV_REQ,
};
static uint8_t servo_stt = 0;
static uint32_t servo_time = 0;

static uint16_t pwm_last[2];
static uint16_t pwm_req[2];
static uint16_t pwm_duration = 0;

#define PWM_NEUTRAL 307		// 1.5ms/20ms*4096 = 307.2
#define PWM_COEF    164		// (2.3ms-1.5ms)/20ms*4096 = 163.8
#define PWM_MIN     143
#define PWM_MAX     471
const struct {uint8_t ledc; uint8_t port;} servoTable[] = {{13,P_SRV0},{14,P_SRV1}};
void _setPwm2(uint8_t idx, int16_t data)
{
	ledcAttachPin(servoTable[idx].port, servoTable[idx].ledc);
	ledcWrite(servoTable[idx].ledc, data);
	pwm_last[idx] = data;
}

static const uint16_t maxOffsetTbl[] = {164,83,70,63,58,54,51,48,45,43,41,40,38,37,35,34,33,32,31,30,29,28,27,26,26,25,24,24,23,22,22,21,21,20,19,19,18,18,17,17,16,16,16,15,15,14,14,14,13,13,12};

int16_t limitOffset(int16_t offset)
{
		 if(offset >= (int) numof(maxOffsetTbl)) offset =   numof(maxOffsetTbl)-1;
	else if(offset <= (int)-numof(maxOffsetTbl)) offset = -((int)numof(maxOffsetTbl)-1);
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
		_setPwm2(0, 0);
		servo_stt = SERVO_REV_REQ;
	} else {
		_setPwm2(0, pwmL);
	}

	if(pwm_last[1] && pwmR && ((pwm_last[1]>=pwm0[1]) != (pwmR>=pwm0[1]))) {
		_setPwm2(1, 0);
		servo_stt = SERVO_REV_REQ;
	} else {
		_setPwm2(1, pwmR);
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
	_setPwm2(0, 0);
	_setPwm2(1, 0);
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
		_setPwm2(0, PWM_NEUTRAL);
		_setPwm2(1, PWM_NEUTRAL);
	} else {
		_setMotor(speed*dir_table[direction].L, speed*dir_table[direction].R, calib, duration);
	}
}

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

void onConnect(String ip)
{
	wsServer.listen(PORT_WEBSOCKET);
	espcamera_onConnect();
	Serial.println(ip);
}

void _setup(const char* ver)
{
	Serial.begin(115200);

	// ServoCar
	ledcSetup(servoTable[0].ledc, 50/*Hz*/, 12/*bit*/);
	ledcSetup(servoTable[1].ledc, 50/*Hz*/, 12/*bit*/);

	espcamera_setup();

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
				_setPwm2(0, pwm_req[0]);
				_setPwm2(1, pwm_req[1]);
				break;

			case SERVO_STOP_REQ:
			default:
				servo_stt = SERVO_IDLE;
				servo_time = 0;
				_setPwm2(0, 0);
				_setPwm2(1, 0);
				break;
			}
		}
	}

	espcamera_loop();
	delay(40);
}
