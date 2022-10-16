/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include "main.h"
#include "panel_def.h"

lgfx::LGFX_Device *lcd = NULL;
WebsocketsServer wsServer;
Preferences preferencesLCD;

const char LcdTypeStr[][16] = {
	"DEFAULT",		// 0
	"SSD1306",
	"SSD1306_32",
	"SSD1331",
	"3248S035",
	"GC9A01",
};


void _setupLCD(int lcdType, uint8_t *config_buf, int config_size)
{
	if(lcd) {
		lcd->releaseBus();
		delete lcd;
		lcd = NULL;
	}

	switch(lcdType) {
	case LCDTYPE_SSD1306:
	case LCDTYPE_SSD1306_32:
		lcd = new LGFX_SSD1306(lcdType, config_buf, config_size);
		break;
	case LCDTYPE_SSD1331:
		lcd = new LGFX_SSD1331(lcdType, config_buf, config_size);
		break;
	case LCDTYPE_3248S035:
		lcd = new LGFX_3248S035(lcdType, config_buf, config_size);
		break;
	case LCDTYPE_GC9A01:
		lcd = new LGFX_GC9A01(lcdType, config_buf, config_size);
		break;
	default:
		return;
	}

	lcd->init();
	lcd->fillScreen(TFT_BLACK);
	lcd->setTextColor(TFT_WHITE,TFT_BLACK);
	lcd->setFont(&fonts::lgfxJapanGothic_12);
	lcd->setCursor(0,0);
	lcd->println("OK");
	lcd->setCursor(0,0);
}

static void onConnect(String ip)
{
	if(lcd) {
		lcd->fillScreen(TFT_BLACK);
		lcd->setCursor(0,0);
		lcd->println(ip);
	}
	wsServer.listen(PORT_WEBSOCKET);
}

void _setup(const char* ver)
{
	Serial.begin(115200);
	do {
		preferencesLCD.begin("lcdConfig", false);

		int lcdType = preferencesLCD.getInt("lcdType", -1);
		if(lcdType < 0) {
			Serial.printf("not configured\n");
			break;
		}

		uint8_t config[NVSCONFIG_MAX];
		memset(config, 0xFF, NVSCONFIG_MAX);
		int config_num = preferencesLCD.getBytes("config", config, NVSCONFIG_MAX);

		Serial.printf("type=%s\n", LcdTypeStr[lcdType]);

		_setupLCD(lcdType, config, config_num);
		lcd->println(ver);
	} while(0);

	initWifi(ver, false, onConnect);
//	Serial.printf("heap_caps_get_free_size(MALLOC_CAP_DMA):%d\n", heap_caps_get_free_size(MALLOC_CAP_DMA));
}

void _loop(void)
{
//	delay(50);
}
