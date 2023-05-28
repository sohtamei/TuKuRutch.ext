/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include "main.h"

#include <TukurutchEspCamera.h>
#include <TukurutchEspCamera.cpp.h>

lgfx::LGFX_Device *lcd = NULL;

#define NVSCONFIG_MAX  12

#include <Preferences.h>

WebsocketsServer wsServer;
Preferences preferencesLCD;

#include <lgfx_panel_def.h>

void _setupLCD(int lcdType, uint8_t *config_buf, int config_size)
{
	if(lcd) {
		lcd->releaseBus();
		delete lcd;
		lcd = NULL;
	}

	switch(lcdType) {
	case LCDTYPE_ROUNDXIAO:
		lcd = new LGFX_ROUNDXIAO(lcdType, config_buf, config_size);
		break;
	case LCDTYPE_SQUAREXIAO:
		lcd = new LGFX_SQUAREXIAO(lcdType, config_buf, config_size);
		lcd->setRotation(3);
		break;
	case LCDTYPE_RoundTouchXIAO:
		lcd = new LGFX_RoundTouchXIAO(lcdType, config_buf, config_size);
		break;
	default:
		return;
	}
	if(lcd->width()==0) {
		Serial.println("error");
		lcd->releaseBus();
		delete lcd;
		lcd = NULL;
		return;
	}

	lcd->fillScreen(TFT_BLACK);
	lcd->setTextColor(TFT_WHITE,TFT_BLACK);
	lcd->setFont(&fonts::lgfxJapanGothic_12);
	lcd->setCursor(0,lcd->height()/3);
	lcd->println("OK");
	lcd->setCursor(0,lcd->height()/3);
}

int _getLcdConfig(uint8_t* buf)
{
	if(!lcd) {
		return 0;
	}

	SetL16(buf+0, lcd->width());
	SetL16(buf+2, lcd->height());
	int lcdType = preferencesLCD.getInt("lcdType", -1);
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
	preferencesLCD.putInt("lcdType", lcdType);
	if(config_size)
		preferencesLCD.putBytes("config", config_buf, config_size);
	else
		preferencesLCD.remove("config");
}

static void onConnect(String ip)
{
	if(lcd) {
		lcd->fillScreen(TFT_BLACK);
		lcd->setCursor(0,lcd->height()/3);
		lcd->println(ip);
	}
	wsServer.listen(PORT_WEBSOCKET);
	espcamera_onConnect();
	Serial.println(ip);
}

void _setup(const char* ver)
{
	Serial.begin(115200);

	espcamera_setup();
	sensor_t * s = esp_camera_sensor_get();
	s->set_vflip(s, 1);
	s->set_hmirror(s, 0);
	do {
		uint8_t config[NVSCONFIG_MAX];
		memset(config, 0xFF, NVSCONFIG_MAX);

		preferencesLCD.begin("lcdConfig", false);
		int lcdType = preferencesLCD.getInt("lcdType", -1);
		if(lcdType < 0) {
			Serial.println("not configured");
			break;
		}
		int config_size = preferencesLCD.getBytes("config", config, NVSCONFIG_MAX);
		Serial.print("type=");
		Serial.println(LcdTypeStr[lcdType]);
		Serial.print("size=");
		Serial.println(config_size);

		_setupLCD(lcdType, config, config_size);
		if(lcd) lcd->println(ver);
	} while(0);

	initWifi(ver, false, onConnect);
}

void _loop(void)
{
	espcamera_loop();
	delay(40);
}
