/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include "main.h"

lgfx::LGFX_Device *lcd = NULL;

#define NVSCONFIG_MAX  12

#if defined(ESP32)
  #include <Preferences.h>

  WebsocketsServer wsServer;
  Preferences preferencesLCD;

#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
  #include <hardware/flash.h>
  #include <hardware/irq.h>
  #include <hardware/sync.h>

  #define FLASH_BASE 0x1F0000
  const uint8_t* flashLCD = (const uint8_t*)(XIP_BASE + FLASH_BASE);
  #define FLASHLCD_LCDTYPE			0x0
  #define FLASHLCD_CONFIG_SIZE		0x2
  #define FLASHLCD_CONFIG			0x4
#else
  #error
#endif

#include "panel_def.h"

void _setupLCD(int lcdType, uint8_t *config_buf, int config_size)
{
	if(lcd) {
		lcd->releaseBus();
		delete lcd;
		lcd = NULL;
	}

	switch(lcdType) {
#if defined(ESP32)
	case LCDTYPE_AUTO:
	case LCDTYPE_AUTO_ROT1:
		lcd = new LGFX();
		lcd->init();
		if(lcdType == LCDTYPE_AUTO_ROT1)
			lcd->setRotation(1);
		break;
#endif
#if defined(ESP32) //|| defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
	// i2c
	case LCDTYPE_SSD1306:
	case LCDTYPE_SSD1306_32:
	case LCDTYPE_SSD1306_6432:	lcd = new LGFX_SSD1306(lcdType, config_buf, config_size); break;
	case LCDTYPE_SH110X:		lcd = new LGFX_SH110X(lcdType, config_buf, config_size); break;
#endif

	// Type1
	case LCDTYPE_085TFT_SPI:	lcd = new LGFX_085TFT_SPI(lcdType, config_buf, config_size); break;
	case LCDTYPE_QT095B:		lcd = new LGFX_QT095B(lcdType, config_buf, config_size); break;
	case LCDTYPE_MSP0961:		lcd = new LGFX_MSP0961(lcdType, config_buf, config_size); break;
	case LCDTYPE_MSP1141:		lcd = new LGFX_MSP1141(lcdType, config_buf, config_size); break;

	case LCDTYPE_MSP1308_GMT130:lcd = new LGFX_MSP1308_GMT130(lcdType, config_buf, config_size); break;
	case LCDTYPE_MSP1541:		lcd = new LGFX_MSP1541(lcdType, config_buf, config_size); break;

	case LCDTYPE_GMT177:		lcd = new LGFX_GMT177(lcdType, config_buf, config_size); break;
	case LCDTYPE_MSP2008:		lcd = new LGFX_MSP2008(lcdType, config_buf, config_size); break;

	// Type2
	case LCDTYPE_MSP1443:		lcd = new LGFX_MSP1443(lcdType, config_buf, config_size); break;
	case LCDTYPE_MSP1803:		lcd = new LGFX_MSP1803(lcdType, config_buf, config_size); break;

	case LCDTYPE_MSP2202:
	case LCDTYPE_MSP2401_2402:
	case LCDTYPE_MSP2806_2807:
	case LCDTYPE_MSP3217_3218:	lcd = new LGFX_MSP2202_240x_280x_321x(lcdType, config_buf, config_size); break;

	case LCDTYPE_MSP3520_3521:	lcd = new LGFX_MSP352x(lcdType, config_buf, config_size); break;
	case LCDTYPE_MSP4020_4021:	lcd = new LGFX_MSP4020_4021(lcdType, config_buf, config_size); break;
	case LCDTYPE_MSP4022_4023:	lcd = new LGFX_MSP4022_4023(lcdType, config_buf, config_size); break;

	// other
	case LCDTYPE_ROUNDLCD:		lcd = new LGFX_ROUNDLCD(lcdType, config_buf, config_size); break;
	case LCDTYPE_ROUNDXIAO:		lcd = new LGFX_ROUNDXIAO(lcdType, config_buf, config_size); break;
	case LCDTYPE_SQUARE:		lcd = new LGFX_SQUARE(lcdType, config_buf, config_size); break;
	case LCDTYPE_128TFT:		lcd = new LGFX_128TFT(lcdType, config_buf, config_size); break;
	case LCDTYPE_ATM0177B3A:	lcd = new LGFX_ATM0177B3A(lcdType, config_buf, config_size); break;

	// micom+LCD
#if defined(CONFIG_IDF_TARGET_ESP32)
	case LCDTYPE_3248S035:		lcd = new LGFX_3248S035(lcdType, config_buf, config_size); break;
	case LCDTYPE_TTGO_TDISP:	lcd = new LGFX_TTGO_TDISP(lcdType, config_buf, config_size); break;
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
	case LCDTYPE_1732S019:		lcd = new LGFX_1732S019(lcdType, config_buf, config_size); break;
#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
	case LCDTYPE_RP2040LCD128:	lcd = new LGFX_RP2040LCD128(lcdType, config_buf, config_size); break;
#endif

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
#if defined(ESP32)
	preferencesLCD.putInt("lcdType", lcdType);
	if(config_size)
		preferencesLCD.putBytes("config", config_buf, config_size);
	else
		preferencesLCD.remove("config");
#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
	uint8_t buf[FLASH_PAGE_SIZE];

	SetL16(buf+FLASHLCD_LCDTYPE, lcdType);
	SetL16(buf+FLASHLCD_CONFIG_SIZE, config_size);
	memcpy(buf+FLASHLCD_CONFIG, config_buf, config_size);

	uint32_t status = save_and_disable_interrupts();
	flash_range_erase(FLASH_BASE, FLASH_SECTOR_SIZE);
	flash_range_program(FLASH_BASE, buf, FLASH_PAGE_SIZE);
	restore_interrupts(status);
#endif
}

#if defined(ESP32)
static void onConnect(String ip)
{
	if(lcd) {
		lcd->fillScreen(TFT_BLACK);
		lcd->setCursor(0,0);
		lcd->println(ip);
	}
	wsServer.listen(PORT_WEBSOCKET);
	Serial.println(ip);
}
#endif

void _setup(const char* ver)
{
	Serial.begin(115200);
	do {
		uint8_t config[NVSCONFIG_MAX];
		memset(config, 0xFF, NVSCONFIG_MAX);

	#if defined(ESP32)
		preferencesLCD.begin("lcdConfig", false);

		int lcdType = preferencesLCD.getInt("lcdType", -1);
		if(lcdType < 0) {
			Serial.println("not configured");
			break;
		}
		int config_size = preferencesLCD.getBytes("config", config, NVSCONFIG_MAX);

	#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
		delay(400);
		int lcdType = GetL16(flashLCD+FLASHLCD_LCDTYPE);
		if(lcdType < 0 || lcdType >= sizeof(LcdTypeStr)/sizeof(LcdTypeStr[0])) break;

		int config_size = GetL16(flashLCD+FLASHLCD_CONFIG_SIZE);
		if(config_size < 0 || config_size > NVSCONFIG_MAX) break;
		memcpy(config, flashLCD+FLASHLCD_CONFIG, config_size);
	#endif
		Serial.print("type=");
		Serial.println(LcdTypeStr[lcdType]);
		Serial.print("size=");
		Serial.println(config_size);

		_setupLCD(lcdType, config, config_size);
		if(lcd) lcd->println(ver);
	} while(0);

#if defined(ESP32)
	initWifi(ver, false, onConnect);
#endif
}

void _loop(void)
{
//	delay(50);
}
