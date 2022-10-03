/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include "main.h"


LGFX_SSD1306 *lcd = NULL;
WebsocketsServer wsServer;

void _initLCD(int sda, int scl, int lcdType)
{
	if(lcd) return;

	lcd = new LGFX_SSD1306(sda,scl,lcdType);
	lcd->init();
	lcd->fillScreen(TFT_BLACK);
	lcd->setTextColor(TFT_WHITE,TFT_BLACK);
	lcd->setFont(&fonts::lgfxJapanGothic_12);
	lcd->setCursor(0,0);
	lcd->println("成功");
}

static void onConnect(String ip)
{
	wsServer.listen(PORT_WEBSOCKET);
}

void _setup(const char* ver)
{
	Serial.begin(115200);
	initWifi(ver, false, onConnect);
//	Serial.printf("heap_caps_get_free_size(MALLOC_CAP_DMA):%d\n", heap_caps_get_free_size(MALLOC_CAP_DMA));
}

void _loop(void)
{
//	delay(50);
}
