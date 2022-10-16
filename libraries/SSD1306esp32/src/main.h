#ifndef main_h
#define main_h

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <TukurutchEsp.h>
#include <Preferences.h>

extern WebsocketsServer wsServer;
extern Preferences preferencesLCD;

void _setup(const char* ver);
void _loop(void);

typedef struct {
	uint8_t sda;
	uint8_t scl;
} nvscfg_i2c_t;

typedef struct {
	uint16_t clk_khz;
	uint8_t sclk;
	uint8_t miso;
	uint8_t mosi;
	uint8_t dc;
	uint8_t cd;
	uint8_t busy;
} nvscfg_spi_t;

enum {
	NVSnvscfg_I2C = sizeof(nvscfg_i2c_t),
	NVSnvscfg_SPI = sizeof(nvscfg_spi_t),

	NVSCONFIG_MAX = 12,
};

void _setupLCD(int lcdType, uint8_t *config_buf, int config_size);

enum {
	LCDTYPE_DEFAULT = 0,
	LCDTYPE_SSD1306 = 1,
	LCDTYPE_SSD1306_32 = 2,
	LCDTYPE_SSD1331 = 3,
	LCDTYPE_3248S035 =4,
};

class LGFX_SSD1306 : public lgfx::LGFX_Device
{
	lgfx::Panel_SSD1306	_panel_instance;
	lgfx::Bus_I2C		_bus_instance;
public:
	LGFX_SSD1306(int lcdType, uint8_t *config_buf, int config_size)
	{
		{ // バス制御の設定
			nvscfg_i2c_t* nvs = (nvscfg_i2c_t*)config_buf;

			auto cfg = _bus_instance.config();
			cfg.i2c_port	= 0;			// 使用するI2Cポートを選択 (0 or 1)
			cfg.freq_write	= 400000;		// 送信時のクロック
			cfg.freq_read	= 400000;		// 受信時のクロック
			cfg.pin_sda		= nvs->sda;		// SDAを接続しているピン番号
			cfg.pin_scl		= nvs->scl;		// SCLを接続しているピン番号
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

class LGFX_SSD1331 : public lgfx::LGFX_Device
{
	lgfx::Panel_SSD1331	_panel_instance;
	lgfx::Bus_SPI		_bus_instance;			// SPIバスのインスタンス
public:
	LGFX_SSD1331(int lcdType, uint8_t *config_buf, int config_size)
	{
		{ // バス制御の設定を行います。
			auto cfg = _bus_instance.config();	// バス設定用の構造体を取得します。

// SPIバスの設定
			cfg.spi_host = SPI2_HOST;			// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			// ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.spi_3wire  = false;				// 受信をMOSIピンで行う場合はtrueを設定
			cfg.use_lock   = true;				// トランザクションロックを使用する場合はtrueを設定
			cfg.dma_channel = SPI_DMA_CH_AUTO;	// 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
			// ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
			cfg.pin_sclk = 32;					// SPIのSCLKピン番号を設定
			cfg.pin_mosi = 33;					// SPIのMOSIピン番号を設定
			cfg.pin_miso = -1;					// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = 26;					// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_instance.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_instance);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs           =   27;	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst          =   25;	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy         =    -1;	// BUSYが接続されているピン番号 (-1 = disable)
			// ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。
			cfg.memory_width     =    96;	// ドライバICがサポートしている最大の幅
			cfg.memory_height    =    64;	// ドライバICがサポートしている最大の高さ
			cfg.panel_width      =    96;	// 実際に表示可能な幅
			cfg.panel_height     =    64;	// 実際に表示可能な高さ
			cfg.offset_x         =     0;	// パネルのX方向オフセット量
			cfg.offset_y         =     0;	// パネルのY方向オフセット量
			cfg.offset_rotation  =     0;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.dummy_read_pixel =     8;	// ピクセル読出し前のダミーリードのビット数
			cfg.dummy_read_bits  =     1;	// ピクセル以外のデータ読出し前のダミーリードのビット数
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = false;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        =  true;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.dlen_16bit       = false;	// データ長を16bit単位で送信するパネルの場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

			_panel_instance.config(cfg);
		}

		setPanel(&_panel_instance); // 使用するパネルをセットします。
	}
};

class LGFX_3248S035 : public lgfx::LGFX_Device
{
	lgfx::Panel_GC9A01	_panel_instance;
	lgfx::Bus_SPI		_bus_instance;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
public:
	LGFX_3248S035(int lcdType, uint8_t *config_buf, int config_size)
	{
		{ // バス制御の設定を行います。
			auto cfg = _bus_instance.config();	// バス設定用の構造体を取得します。

// SPIバスの設定
			cfg.spi_host = SPI2_HOST;			// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			// ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.spi_3wire  = false;				// 受信をMOSIピンで行う場合はtrueを設定
			cfg.use_lock   = true;				// トランザクションロックを使用する場合はtrueを設定
			cfg.dma_channel = SPI_DMA_CH_AUTO;	// 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
			// ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
			cfg.pin_sclk = 14;					// SPIのSCLKピン番号を設定
			cfg.pin_mosi = 13;					// SPIのMOSIピン番号を設定
			cfg.pin_miso = 12;					// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   =  2;					// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_instance.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_instance);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs           =    15;	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst          =    -1;	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy         =    -1;	// BUSYが接続されているピン番号 (-1 = disable)
			// ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。
			cfg.memory_width     =   320;	// ドライバICがサポートしている最大の幅
			cfg.memory_height    =   480;	// ドライバICがサポートしている最大の高さ
			cfg.panel_width      =   320;	// 実際に表示可能な幅
			cfg.panel_height     =   480;	// 実際に表示可能な高さ
			cfg.offset_x         =     0;	// パネルのX方向オフセット量
			cfg.offset_y         =     0;	// パネルのY方向オフセット量
			cfg.offset_rotation  =     0;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.dummy_read_pixel =     8;	// ピクセル読出し前のダミーリードのビット数
			cfg.dummy_read_bits  =     1;	// ピクセル以外のデータ読出し前のダミーリードのビット数
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = false;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.dlen_16bit       = false;	// データ長を16bit単位で送信するパネルの場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

			_panel_instance.config(cfg);
		}
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。

			cfg.pin_bl = 27;				// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = 7;			// 使用するPWMのチャンネル番号

			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
		setPanel(&_panel_instance); // 使用するパネルをセットします。
	}
};

extern lgfx::LGFX_Device *lcd;

#endif  // main_h
