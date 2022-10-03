#ifndef main_h
#define main_h

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <TukurutchEsp.h>

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);
void _initLCD(int sda, int scl, int lcdType);

enum {
	LCDTYPE_DEFAULT = 0,
	LCDTYPE_SSD1306_32 = 1,
};

class LGFX_SSD1306 : public lgfx::LGFX_Device
{
	lgfx::Panel_SSD1306	_panel_instance;
	lgfx::Bus_I2C		_bus_instance;
public:
	LGFX_SSD1306(int sda, int scl, int lcdType)
	{
		{ // バス制御の設定
			auto cfg = _bus_instance.config();
			cfg.i2c_port	= 0;			// 使用するI2Cポートを選択 (0 or 1)
			cfg.freq_write	= 400000;		// 送信時のクロック
			cfg.freq_read	= 400000;		// 受信時のクロック
			cfg.pin_sda		= sda;			// SDAを接続しているピン番号
			cfg.pin_scl		= scl;			// SCLを接続しているピン番号
			cfg.i2c_addr	= 0x3C;			// I2Cデバイスのアドレス
			_bus_instance.config(cfg);		// 設定値をバスに反映
			_panel_instance.setBus(&_bus_instance);
			if(lcdType==LCDTYPE_SSD1306_32)
				_panel_instance.setComPins(0x02);	// 0x12 for 128x64, 0x02 for 128x32
		}

		{ // 表示パネル制御の設定
			auto cfg = _panel_instance.config();
			cfg.panel_width		= 128;		// 実際に表示可能な幅
			cfg.panel_height	= (lcdType==LCDTYPE_SSD1306_32)?32:64;
											// 実際に表示可能な高さ
			_panel_instance.config(cfg);
		}

		setPanel(&_panel_instance);	// 使用するパネルをセット
	}
};

extern LGFX_SSD1306 *lcd;

#endif  // main_h
