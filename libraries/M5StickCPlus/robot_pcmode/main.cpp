/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include <TukurutchEsp.h>
#include "main.h"

// ポート定義

#define P_LED			10

WebsocketsServer wsServer;

#define numof(a) (sizeof(a)/sizeof((a)[0]))

void _setLED(uint8_t onoff)
{
	digitalWrite(P_LED, !onoff);
	pinMode(P_LED, OUTPUT);
}

uint8_t _getSw(uint8_t button)
{
	switch(button) {
	case 0: return M5.BtnA.isPressed();
	case 1: return M5.BtnB.isPressed();
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
#ifdef _M5StickCPlus
	M5.Beep.tone(sound, ms);
	delay(ms);
	M5.Beep.mute();
#else
	delay(ms);
#endif
}

void _beep(void)
{
#ifdef _M5StickCPlus
	M5.Beep.beep();
	delay(100);
	M5.Beep.mute();
#endif
}

static void onConnect(String ip)
{
	M5.Lcd.fillScreen(BLACK);
	M5.Lcd.setCursor(0,0);
	M5.Lcd.println(ip);
	wsServer.listen(PORT_WEBSOCKET);
}

void _setup(const char* ver)
{
	M5.begin(true, true, true); // init lcd, power, serial
	M5.IMU.Init();
	M5.Lcd.setRotation(3);

	M5.Lcd.fillScreen(BLACK);
	M5.Lcd.setTextSize(2);

	M5.Lcd.setCursor(0, 0);
	if(_getSw(0)) {
		M5.Lcd.println("ESP SmartConfig");
		WiFi.mode(WIFI_STA);
		WiFi.beginSmartConfig();
		while (!WiFi.smartConfigDone()) {
			delay(1000);
			_setLED(1);
			_tone(T_C5, 100);
			_setLED(0);
		}
	} else {
		M5.Lcd.println(ver);
	}

	Serial.begin(115200);
	initWifi(ver, false, onConnect);
}

void _loop(void)
{
	M5.update();  // update button and speaker
//	delay(50);
}
