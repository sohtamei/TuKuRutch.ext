/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include "main.h"

#include <TukurutchEspCamera.h>
#include <TukurutchEspCamera.cpp.h>

WebsocketsServer wsServer;

#include "panel_def.h"

lgfx::LGFX_Device *lcd = NULL;

int _getLcdConfig(uint8_t* buf)
{
	SetL16(buf+0, lcd->width());
	SetL16(buf+2, lcd->height());
	int lcdType = LCDTYPE_SQUARE;
	SetL16(buf+4, lcdType);
	return 6;
}

static void onConnect(String ip)
{
	if(!lcd) return;
	lcd->fillScreen(TFT_BLACK);
	lcd->setCursor(0,0);
	lcd->println(ip);
	wsServer.listen(PORT_WEBSOCKET);
	espcamera_onConnect();
	Serial.println(ip);
}

void _setup(const char* ver)
{
	Serial.begin(115200);
	espcamera_setup();
	lcd = new LGFX_SQUARE(0, NULL, 0);
	lcd->fillScreen(TFT_BLACK);
	lcd->setTextColor(TFT_WHITE,TFT_BLACK);
	lcd->setFont(&fonts::lgfxJapanGothic_12);
	lcd->setCursor(0,0);
	lcd->println(ver);

	initWifi(ver, false, onConnect);
}

void _loop(void)
{
	espcamera_loop();
	delay(40);
}
