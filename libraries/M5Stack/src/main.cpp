/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include <TukurutchEsp.h>
#include "main.h"


LGFX lcd;
WebsocketsServer wsServer;

#define numof(a) (sizeof(a)/sizeof((a)[0]))

void _setLED(uint8_t onoff)
{;}

uint8_t _getSw(uint8_t button)
{
	switch(button) {
	case 0: return M5.BtnA.isPressed();
	case 1: return M5.BtnB.isPressed();
	case 2: return M5.BtnC.isPressed();
	}
	return 0;
}

float getIMU(uint8_t index)
{
	float data[3] = {0};
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

static void onConnect(String ip)
{
	lcd.fillScreen(TFT_BLACK);
	lcd.setCursor(0,0);
	lcd.println(ip);
	wsServer.listen(PORT_WEBSOCKET);
}

void _setup(const char* ver)
{
	M5.begin(false, true, true); // init lcd, sd card, serial
	M5.Power.begin();    // use battery
//	M5.IMU.Init();
	lcd.init();
	Serial.printf("heap_caps_get_free_size(MALLOC_CAP_DMA):%d\n", heap_caps_get_free_size(MALLOC_CAP_DMA));

	lcd.fillScreen(TFT_BLACK);
	lcd.setTextSize(2);

	lcd.setCursor(0, 0);
	lcd.println(ver);

	initWifi(ver, false, onConnect);
}

void _loop(void)
{
	M5.update();  // update button and speaker
//	delay(50);
}
