/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include <TukurutchEsp.h>
#include <M5Atom.h>
#include "main.h"

// ポート定義

WebsocketsServer wsServer;

#define numof(a) (sizeof(a)/sizeof((a)[0]))

void _setLED(uint8_t onoff)
{
	if(onoff)
		M5.dis.drawpix(0,0xf00000);
	else
		M5.dis.clear();
}

uint8_t _getSw(uint8_t button)
{
	switch(button) {
	case 0: return M5.Btn.isPressed();
//	case 1: return M5.BtnB.isPressed();
//	case 2: return M5.BtnC.isPressed();
	}
	return 0;
}

float getIMU(uint8_t index)
{
	float data[3]={0};
	if(index < 3) {
		M5.IMU.getGyroData(data+0,data+1,data+2);
		return data[index-0];
	} else if(index < 6) {
		M5.IMU.getAccelData(data+0,data+1,data+2);
		return data[index-3];
	} else if(index < 9) {
		//M5.IMU.getAhrsData(data+0,data+1,data+2);
		return data[index-6];
	} else {
		M5.IMU.getTempData(data+0);
		return data[0];
	}
}

void _tone(int sound, int ms)
{
	delay(ms);
}

// ServoCar

const struct {uint8_t ledc; uint8_t port;} servoTable[] = {{8,32},{9,26}};
void _setServo(uint8_t idx, int16_t data/*normal:0~180, continuous:-100~+100*/, uint8_t continuous)
{
	if(idx >= numof(servoTable)) return;

	uint16_t pwmWidth;
	if(continuous) {
		#define srvZero 307		// 1.5ms/20ms*4096 = 307.2
		#define srvCoef 163		// (2.3ms-1.5ms)/20ms*4096 = 163.8
		if(data < -100) data = -100;
		else if(data > 100) data = 100;
		if(idx == 1) data = -data;
		pwmWidth = (data * srvCoef) / 100 + srvZero;
		if(data==0 && continuous!=2) pwmWidth=0;
	} else {
		#define srvMin 103		// 0.5ms/20ms*4096 = 102.4 (-90c)
		#define srvMax 491		// 2.4ms/20ms*4096 = 491.5 (+90c)
		if(data < 0) data = 0;
		else if(data > 180) data = 180;
		pwmWidth = (data * (srvMax - srvMin)) / 180 + srvMin;
	}
	ledcAttachPin(servoTable[idx].port, servoTable[idx].ledc);
	ledcWrite(servoTable[idx].ledc, pwmWidth);
}

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

void _setCar(uint8_t direction, uint8_t speed)
{
	if(direction >= numof(dir_table)) {
		_setServo(0, 0, 2);
		_setServo(1, 0, 2);
	} else {
		_setServo(0, speed * dir_table[direction].L, 1);
		_setServo(1, speed * dir_table[direction].R, 1);
	}
}

static void onConnect(String ip)
{
	_setLED(1);
	wsServer.listen(PORT_WEBSOCKET);
	Serial.println(ip);
}

void _setup(const char* ver)
{
	M5.begin(true, true, true); // init lcd, power, serial
	M5.IMU.Init();
	Serial.begin(115200);
	if(_getSw(0)) {
		Serial.println("Waiting for SmartConfig.");
		WiFi.mode(WIFI_STA);
		WiFi.beginSmartConfig();
		while (!WiFi.smartConfigDone()) {
			delay(1000);
			_setLED(1);
			_tone(T_C5, 100);
			_setLED(0);
		}
		Serial.println("SmartConfig received.");
	}

	// ServoCar
	ledcSetup(servoTable[0].ledc, 50/*Hz*/, 12/*bit*/);
	ledcSetup(servoTable[1].ledc, 50/*Hz*/, 12/*bit*/);

	initWifi(ver, false, onConnect);
}

void _loop(void)
{
//	loopWebSocket();
	M5.update();  // update button and speaker
//	delay(50);
}
