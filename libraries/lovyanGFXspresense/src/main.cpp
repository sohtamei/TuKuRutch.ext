/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include "main.h"

LGFX lcdInstance;
lgfx::LGFX_Device *lcd = &lcdInstance;

void _setupLCD(int lcdType, uint8_t *config_buf, int config_size)
{
	lcd->init();
	lcd->setRotation(1);
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
#if defined(ESP32)
	int lcdType = preferencesLCD.getInt("lcdType", -1);
#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
	int lcdType = GetL16(flashLCD+FLASHLCD_LCDTYPE);
#else
	int lcdType = 0;
#endif
	SetL16(buf+4, lcdType);

	lgfx::IBus* bus = lcd->panel()->bus();
	lgfx::ILight* light = lcd->panel()->light();

	switch(bus->busType()) {
		default:
			return 6;
	#if defined(ESP32)
		case lgfx::bus_i2c: {
			auto cfgI2c = ((lgfx::Bus_I2C*)bus)->config();
			buf[6] = cfgI2c.pin_sda;
			buf[7] = cfgI2c.pin_scl;
			return 8;
		}
	#endif
		case lgfx::bus_spi: {
			auto cfgSpi = ((lgfx::Bus_SPI*)bus)->config();
			buf[6] = cfgSpi.pin_sclk;
			buf[7] = cfgSpi.pin_mosi;
			buf[8] = cfgSpi.pin_miso;
			buf[9] = cfgSpi.pin_dc;

			auto cfgPanel = lcd->panel()->config();
			buf[10] = cfgPanel.pin_cs;
			buf[11] = cfgPanel.pin_rst;
			buf[12] = cfgPanel.pin_busy;

			if(light) {
			#if defined(ESP32)
				auto cfgPwm = ((lgfx::Light_PWM*)light)->config();
				buf[13] = cfgPwm.pin_bl;
			#endif
			} else {
				buf[13] = -1;
			}
			return 14;
		}
	}
}

void _setLcdConfig(int lcdType, uint8_t *config_buf, int config_size)
{
}

void _setup(const char* ver)
{
	Serial.begin(115200);
	do {
		Serial.println("type=spresence");
		_setupLCD(0,NULL,0);
		if(lcd) lcd->println(ver);
	} while(0);
}

void _loop(void)
{
//	delay(50);
}
