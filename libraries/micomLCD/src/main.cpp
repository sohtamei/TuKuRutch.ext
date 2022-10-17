/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include "main.h"

lgfx::LGFX_Device *lcd = NULL;
WebsocketsServer wsServer;
Preferences preferencesLCD;

#include "panel_def.h"

void _setupLCD(int lcdType, uint8_t *config_buf, int config_size)
{
	if(lcd) {
		lcd->releaseBus();
		delete lcd;
		lcd = NULL;
	}

	switch(lcdType) {
	case LCDTYPE_AUTO:
	case LCDTYPE_AUTO_ROLL:
		lcd = new LGFX();
		lcd->init();
		if(lcdType == LCDTYPE_AUTO_ROLL)
			lcd->setRotation(1);
		break;

	case LCDTYPE_SSD1306:
	case LCDTYPE_SSD1306_32:	lcd = new LGFX_SSD1306(lcdType, config_buf, config_size); break;
	case LCDTYPE_SSD1331:		lcd = new LGFX_SSD1331(lcdType, config_buf, config_size); break;
	case LCDTYPE_3248S035:		lcd = new LGFX_3248S035(lcdType, config_buf, config_size); break;
	case LCDTYPE_ROUNDLCD:		lcd = new LGFX_ROUNDLCD(lcdType, config_buf, config_size); break;
	case LCDTYPE_MSP2807:		lcd = new LGFX_MSP2807(lcdType, config_buf, config_size); break;
	default:
		return;
	}

	lcd->fillScreen(TFT_BLACK);
	lcd->setTextColor(TFT_WHITE,TFT_BLACK);
	lcd->setFont(&fonts::lgfxJapanGothic_12);
	lcd->setCursor(0,0);
	lcd->println("OK");
	lcd->setCursor(0,0);
}

int _getLcdConfig(uint8_t* buf)
{
	if(!lcd) {
		return 0;
	}

	SetL16(buf+0, lcd->width());
	SetL16(buf+2, lcd->height());
	buf[4] = preferencesLCD.getInt("lcdType", -1);

	lgfx::IBus* bus = lcd->panel()->bus();
	lgfx::ILight* light = lcd->panel()->light();

	switch(bus->busType()) {
	default:
		return 5;
	case lgfx::bus_i2c: {
		auto cfgI2c = ((lgfx::Bus_I2C*)bus)->config();
		buf[5] = cfgI2c.pin_sda;
		buf[6] = cfgI2c.pin_scl;
		return 7;
	  }
	case lgfx::bus_spi: {
		auto cfgSpi = ((lgfx::Bus_SPI*)bus)->config();
		buf[5] = cfgSpi.pin_sclk;
		buf[6] = cfgSpi.pin_mosi;
		buf[7] = cfgSpi.pin_miso;
		buf[8] = cfgSpi.pin_dc;

		auto cfgPanel = lcd->panel()->config();
		buf[9] = cfgPanel.pin_cs;
		buf[10] = cfgPanel.pin_rst;
		buf[11] = cfgPanel.pin_busy;

		if(light) {
			auto cfgPwm = ((lgfx::Light_PWM*)light)->config();
			buf[12] = cfgPwm.pin_bl;
		} else {
			buf[12] = -1;
		}
		return 13;
	  }
	}
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
		if(lcd) lcd->println(ver);
	} while(0);

	initWifi(ver, false, onConnect);
//	Serial.printf("heap_caps_get_free_size(MALLOC_CAP_DMA):%d\n", heap_caps_get_free_size(MALLOC_CAP_DMA));
}

void _loop(void)
{
//	delay(50);
}
