/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include "main.h"


LGFX_M5Stamp_SPI_GC9A01 lcd;
WebsocketsServer wsServer;

#define numof(a) (sizeof(a)/sizeof((a)[0]))

void _setLED(uint8_t onoff)
{;}

static void onConnect(String ip)
{
	lcd.fillScreen(TFT_BLACK);
	lcd.setCursor(0,0);
	lcd.println(ip);
	wsServer.listen(PORT_WEBSOCKET);
}

void _setup(const char* ver)
{
	Serial.begin(115200);
	lcd.init();
	lcd.setBrightness(128);
	lcd.fillScreen(TFT_BLACK);
	lcd.setTextSize(2);
/*
	lcd.width(), lcd.height()
	lcd.startWrite();
	lcd.endWrite();
*/
	Serial.printf("heap_caps_get_free_size(MALLOC_CAP_DMA):%d\n", heap_caps_get_free_size(MALLOC_CAP_DMA));

	lcd.setCursor(0, 0);
	lcd.println(ver);
	initWifi(ver, false, onConnect);
}

void _loop(void)
{
//	delay(50);
}
