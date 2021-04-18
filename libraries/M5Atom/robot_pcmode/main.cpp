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

	initWifi(ver, false, onConnect);
}

void _loop(void)
{
//	loopWebSocket();
	M5.update();  // update button and speaker
//	delay(50);
}
