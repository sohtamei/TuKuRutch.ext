/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include "main.h"

lgfx::LGFX_Device *lcd = NULL;

#define NVSCONFIG_MAX  12
#include <Preferences.h>
Preferences preferencesLCD;

#if defined(ESP32)
  WebsocketsServer wsServer;
  #define USE_SD
  int8_t pin_sdcs = -1;
  int8_t pin_sdclk = -1;
  int8_t pin_sdmosi = -1;
  int8_t pin_sdmiso = -1;
#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)

#else
  #error
#endif

#include <lgfx_panel_def.h>

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

	case LCDTYPE_128TFT:		lcd = new LGFX_128TFT(lcdType, config_buf, config_size); break;

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

	// XIAO
	case LCDTYPE_ROUNDXIAO:		lcd = new LGFX_ROUNDXIAO(lcdType, config_buf, config_size); break;
	case LCDTYPE_SQUAREXIAO:	lcd = new LGFX_SQUAREXIAO(lcdType, config_buf, config_size); break;
	case LCDTYPE_RoundTouchXIAO: lcd = new LGFX_RoundTouchXIAO(lcdType, config_buf, config_size); break;

	// other
	case LCDTYPE_ROUNDLCD:		lcd = new LGFX_ROUNDLCD(lcdType, config_buf, config_size); break;
	case LCDTYPE_SQUARELCD:		lcd = new LGFX_SQUARELCD(lcdType, config_buf, config_size); break;

	// micom+LCD
#if defined(CONFIG_IDF_TARGET_ESP32)
	case LCDTYPE_3248S035:		lcd = new LGFX_3248S035(lcdType, config_buf, config_size); break;
	case LCDTYPE_TTGO_TDISP:	lcd = new LGFX_TTGO_TDISP(lcdType, config_buf, config_size); break;
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
	case LCDTYPE_ESP32C3_144:	lcd = new LGFX_ESP32C3_144(lcdType, config_buf, config_size); break;
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
	case LCDTYPE_1732S019:		lcd = new LGFX_1732S019(lcdType, config_buf, config_size); break;
#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
	case LCDTYPE_RP2040LCD128:	lcd = new LGFX_RP2040LCD128(lcdType, config_buf, config_size); break;
	case LCDTYPE_RP2040GEEK:	lcd = new LGFX_RP2040GEEK(lcdType, config_buf, config_size); break;
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

#if defined(ESP32)
#if defined(CONFIG_IDF_TARGET_ESP32)
  SPIClass spiSD(VSPI);
#endif

static int sd_initialized = false;
int _beginSD(void)
{
	if(pin_sdcs < 0) return -1;
	if(sd_initialized) return 0;
	SPIClass* spi = &SPI;
  #if defined(CONFIG_IDF_TARGET_ESP32)
	if(pin_sdclk >= 0) {
		spiSD.begin(pin_sdclk, pin_sdmiso, pin_sdmosi, pin_sdcs);
		spi = &spiSD;
	}
  #endif
	for(int i = 0; i < 2; i++) {
		if(SD.begin(pin_sdcs, *spi, 15000000)) {
			sd_initialized = true;
			return 0;
		}
		delay(500);
	}
	return -1;
}

extern char strBuf[256];	// TukurutchEsp.cpp

char* _getFilelist(void)
{
	memset(strBuf, 0, sizeof(strBuf));

	if(_beginSD() < 0) return strBuf;

	File root = SD.open("/");
	if(!root) return strBuf;

	int cnt = 0;
	File file = root.openNextFile();
	while(file) {
		if(!file.isDirectory()) {
			String filename = file.name();
			if (filename.indexOf(".jpg") != -1 || filename.indexOf(".png") != -1 ) {
				int len = strlen(filename.c_str());
				if(cnt+len+1 > 256-1) break;

				memcpy(strBuf+cnt, filename.c_str(), len);
				strBuf[cnt+len] = '\t';
				cnt += len+1;
			}
		}
		file = root.openNextFile();
	}
	if(cnt != 0)
		strBuf[cnt-1] = 0x00;		// last \t
	return strBuf;
}

int _removeFiles(void)
{
	if(_beginSD() < 0) return -1;

	File root = SD.open("/");
	if(!root) return -1;

	File file = root.openNextFile();
	while(file) {
		if(!file.isDirectory()) {
			SD.remove(String("/")+file.name());
		}
		file = root.openNextFile();
	}
	return 0;
}

uint16_t autoDur = 0;
String autoMode = "off";
File autoRoot;
int32_t autoLast = 0;

void _setAutomode(const char* filename, int duration)
{
	preferencesLCD.putString("autoMode", filename);
	preferencesLCD.putUShort("autoDur", duration);
}

void initAutomode(void)
{
	if(_beginSD() < 0) return;

	autoMode = preferencesLCD.getString("autoMode", "off");
	autoDur = preferencesLCD.getUShort("autoDur", 0);
	autoLast = millis();
	Serial.printf("autoMode=%s\n", autoMode);

	if(autoMode == "slideshow") {
		autoRoot = SD.open("/");
	}
}

std::vector<std::string> split_naive(const std::string &s, char delim)
{
	std::vector<std::string> elems;
	std::string item;
	for(char ch: s) {
		if (ch == delim) {
			if (!item.empty())
				elems.push_back(item);
			item.clear();
		}
		else {
			item += ch;
		}
	}
	if(!item.empty())
		elems.push_back(item);
	return elems;
}

void _drawFile(const char* filename, int x, int y)
{
	if(_beginSD() < 0) return;
	if(strstr(filename, ".jpg"))
		lcd->drawJpgFile(SD, filename, x, y, lcd->width(), lcd->height());
	else if(strstr(filename, ".png"))
		lcd->drawPngFile(SD, filename, x, y, lcd->width(), lcd->height());
}

void loopAutomode(void)
{
	int32_t elapsed = ((millis() - autoLast) & 0x7FFFFFFF);

	if(autoMode == "off") {
		;
	} else if(autoMode != "slideshow") {
		if(elapsed >= 1000) {
			_drawFile(autoMode.c_str(), 0, 0);
			autoMode = "off";
		}
	} else {
		if(elapsed >= autoDur*100 && autoRoot) {
			File file = autoRoot.openNextFile();
			if(!file) {
				autoRoot.rewindDirectory();
				file = autoRoot.openNextFile();
			}
			if(file && !file.isDirectory()) {
				int x = 0; int y = 0;
				std::vector<std::string> names = split_naive(file.name(), '.');
				if(names.size() >= 4) {
					try {
						x = stoi(names[1]);
						y = stoi(names[2]);
					} catch(std::exception const& e) {
						Serial.printf("Error: %s\n", e.what());
					}
				}
				_drawFile((String("/")+file.name()).c_str(), x, y);
			}
			autoLast = millis();
		}
	}
}

static void onConnect(String ip)
{
	if(lcd) {
		lcd->fillScreen(TFT_BLACK);
		lcd->setCursor(0,lcd->height()/3);
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

#if defined(ESP32)
	initWifi(ver, false, onConnect);
	initAutomode();
#endif
}

void _loop(void)
{
#if defined(ESP32)
	if(comMode == 0) {			// when comMode = MODE_INVALID
		loopAutomode();
	}
#endif
//	delay(50);
}
