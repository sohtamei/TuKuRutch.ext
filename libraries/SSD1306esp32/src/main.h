#ifndef main_h
#define main_h

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <TukurutchEsp.h>

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);
void _initLCD(int sda, int scl, int width, int height);

class LGFX_SSD1306 : public lgfx::LGFX_Device
{
	lgfx::Panel_SSD1306	_panel_instance;
	lgfx::Bus_I2C		_bus_instance;
public:
	LGFX_SSD1306(int sda, int scl, int width, int height)
	{
		{ // バス制御の設定
			auto cfg = _bus_instance.config();	// バス設定用の構造体を取得
			cfg.i2c_port	= 0;			// 使用するI2Cポートを選択 (0 or 1)
			cfg.freq_write	= 400000;		// 送信時のクロック
			cfg.freq_read	= 400000;		// 受信時のクロック
			cfg.pin_sda		= sda;			// SDAを接続しているピン番号
			cfg.pin_scl		= scl;			// SCLを接続しているピン番号
			cfg.i2c_addr	= 0x3C;			// I2Cデバイスのアドレス
			_bus_instance.config(cfg);		// 設定値をバスに反映
			_panel_instance.setBus(&_bus_instance);
		}

		{ // 表示パネル制御の設定
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得
			cfg.memory_width	= 128;		// ドライバICがサポートしている最大の幅
			cfg.memory_height	= 64;		// ドライバICがサポートしている最大の高さ
			cfg.panel_width		= width;	// 実際に表示可能な幅
			cfg.panel_height	= height;	// 実際に表示可能な高さ
			_panel_instance.config(cfg);
		}

		setPanel(&_panel_instance);	// 使用するパネルをセット
	}
};

extern LGFX_SSD1306 *lcd;

#endif  // main_h
