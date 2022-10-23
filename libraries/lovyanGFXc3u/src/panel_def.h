enum {
	LCDTYPE_NOLCD = 0,
	LCDTYPE_AUTO = 1,
	LCDTYPE_AUTO_ROT1 = 2,

	LCDTYPE_SSD1306 = 3,
	LCDTYPE_SSD1306_32 = 4,
	LCDTYPE_SSD1331 = 5,
	LCDTYPE_3248S035 = 6,
	LCDTYPE_ROUNDLCD = 7,
	LCDTYPE_MSP2807 = 8,
	LCDTYPE_ATM0177B3A = 9,
};

const char LcdTypeStr[][16] = {
	"no-LCD",		// 0
	"AUTO",
	"AUTO_ROT1",
	"SSD1306",
	"SSD1306_32",
	"SSD1331",
	"3248S035",
	"ROUNDLCD",
	"MSP2807",
	"ATM0177B3A",
};

typedef struct {
	uint8_t sda;
	uint8_t scl;
} nvscfg_i2c_t;

typedef struct {
	uint8_t sclk;
	uint8_t mosi;
	uint8_t miso;
	uint8_t dc;
	uint8_t cs;
	uint8_t rst;
	uint8_t busy;
	uint8_t bl;
} nvscfg_spi_t;

#if defined(CONFIG_IDF_TARGET_ESP32)
  #define PWM_CH  7
  int ChkFF(int a) {
    const uint8_t availables[] = {/*input*/34,35, 32,33,25,26,27,14,12, 13,15,2, 0,4,16,17,5,18,19,21,22,23};
    for(int i=0; i<sizeof(availables); i++)
      if(a == availables[i]) return a;
    return -1;
  }
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
  #define PWM_CH  3
  int ChkFF(int a) {
    const uint8_t availables[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17, /*usb*/18,19, /*uart 20,21*/};
    for(int i=0; i<sizeof(availables); i++)
      if(a == availables[i]) return a;
    return -1;
  }
#else
  #define PWM_CH  7
  int ChkFF(int a) {
    const uint8_t availables[] = {4,5,6,7,15,16,17,18,8,19,20, 3,46,9,10,11,12,13,14,21,47,48,45, 0,35,36,19,37,38,39,40,41,42,2,1};
    for(int i=0; i<sizeof(availables); i++)
      if(a == availables[i]) return a;
    return -1;
  }
#endif

class LGFX_SSD1306 : public lgfx::LGFX_Device
{
	lgfx::Panel_SSD1306	_panel_instance;
	lgfx::Bus_I2C		_bus_instance;
public:
	LGFX_SSD1306(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if defined(CONFIG_IDF_TARGET_ESP32)
		nvscfg_i2c_t nvs = {
			.sda = 21,
			.scl = 22,
		};
	#elif defined(CONFIG_IDF_TARGET_ESP32S3)
		nvscfg_i2c_t nvs = {
			.sda = 8,
			.scl = 9,
		};
	#elif defined(CONFIG_IDF_TARGET_ESP32C3)
		nvscfg_i2c_t nvs = {
			.sda = 6,
			.scl = 7,
		};
	#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
		nvscfg_i2c_t nvs = {
			.sda = 4,
			.scl = 5,
		};
	#else
		nvscfg_i2c_t nvs = {0};
		return;
	#endif
		if(config_size >= sizeof(nvscfg_i2c_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		Serial.printf("sda=%d,scl=%d\n", nvs.sda, nvs.scl);
		if(ChkFF(nvs.sda)<0 || ChkFF(nvs.scl)<0) return;

		{ // バス制御の設定
			auto cfg = _bus_instance.config();
			cfg.i2c_port	= 0;				// 使用するI2Cポートを選択 (0 or 1)
			cfg.freq_write	= 400000;			// 送信時のクロック
			cfg.freq_read	= 400000;			// 受信時のクロック
			cfg.pin_sda		= ChkFF(nvs.sda);	// SDAを接続しているピン番号
			cfg.pin_scl		= ChkFF(nvs.scl);	// SCLを接続しているピン番号
			cfg.i2c_addr	= 0x3C;				// I2Cデバイスのアドレス
			_bus_instance.config(cfg);			// 設定値をバスに反映
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
		init();
	}
};

// SSD1331
class LGFX_SSD1331 : public lgfx::LGFX_Device
{
	lgfx::Panel_SSD1331	_panel_instance;
	lgfx::Bus_SPI		_bus_instance;			// SPIバスのインスタンス
public:
	LGFX_SSD1331(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if defined(CONFIG_IDF_TARGET_ESP32)
		nvscfg_spi_t nvs = {
			.sclk = 25,
			.mosi = 26,
			.miso = -1,
			.dc   = 14,
			.cs   = 12,
			.rst  = 27,
			.busy = -1,
			.bl   = -1,
		};
	#elif defined(CONFIG_IDF_TARGET_ESP32S3)
		nvscfg_spi_t nvs = {
			.sclk = 9,
			.mosi = 10,
			.miso = -1,
			.dc   = 12,
			.cs   = 13,
			.rst  = 11,
			.busy = -1,
			.bl   = -1,
		};
	#elif defined(CONFIG_IDF_TARGET_ESP32C3)
		nvscfg_spi_t nvs = {
			.sclk = 5,
			.mosi = 6,
			.miso = -1,
			.dc   = 8,
			.cs   = 10,
			.rst  = 27,
			.busy = -1,
			.bl   = -1,
		};
	#else
		nvscfg_spi_t nvs = {0};
		return;
	#endif
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		Serial.printf("sclk=%d,mosi=%d,miso=%d,dc=%d,cs=%d,rst=%d,busy=%d,bl=%d\n",
					nvs.sclk, nvs.mosi, nvs.miso, nvs.dc, nvs.cs, nvs.rst, nvs.busy, nvs.bl);
		if(ChkFF(nvs.sclk)<0 || ChkFF(nvs.mosi)<0) return;

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
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_instance.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_instance);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
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
		init();
	}
};

// 3248S035
class LGFX_3248S035 : public lgfx::LGFX_Device
{
	lgfx::Panel_GC9A01	_panel_instance;
	lgfx::Bus_SPI		_bus_instance;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
public:
	LGFX_3248S035(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if !defined(CONFIG_IDF_TARGET_ESP32)
		return;
	#endif
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
			cfg.offset_rotation  =     5;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
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
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
		setTextSize(2);
	}
};

// round LCD
class LGFX_ROUNDLCD : public lgfx::LGFX_Device
{
	lgfx::Panel_GC9A01	_panel_instance;
	lgfx::Bus_SPI		_bus_instance;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
public:
	LGFX_ROUNDLCD(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if defined(CONFIG_IDF_TARGET_ESP32)
		nvscfg_spi_t nvs = {
			.sclk = 14,
			.mosi = 12,
			.miso = -1,
			.dc   = 16,
			.cs   = 17,
			.rst  = 15,
			.busy = -1,
			.bl   = 13,
		};
	#elif defined(CONFIG_IDF_TARGET_ESP32S3)
		nvscfg_spi_t nvs = {0};
		return;
	#elif defined(CONFIG_IDF_TARGET_ESP32C3)
		nvscfg_spi_t nvs = {
			.sclk = 4,
			.mosi = 6,
			.miso = -1,
			.dc   = 1,
			.cs   = 7,
			.rst  = 0,
			.busy = -1,
			.bl   = 10,
		};
	#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
		nvscfg_spi_t nvs = {
			.sclk = 18,
			.mosi = 19,
			.miso = -1,
			.dc   = 11,
			.cs   = 13,
			.rst  = 12,
			.busy = -1,
			.bl   = 10,
		};
	#else
		nvscfg_spi_t nvs = {0};
		return;
	#endif
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		Serial.printf("sclk=%d,mosi=%d,miso=%d,dc=%d,cs=%d,rst=%d,busy=%d,bl=%d\n",
					nvs.sclk, nvs.mosi, nvs.miso, nvs.dc, nvs.cs, nvs.rst, nvs.busy, nvs.bl);
		if(ChkFF(nvs.sclk)<0 || ChkFF(nvs.mosi)<0) return;

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
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_instance.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_instance);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			// ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。
			cfg.memory_width     =   240;	// ドライバICがサポートしている最大の幅
			cfg.memory_height    =   240;	// ドライバICがサポートしている最大の高さ
			cfg.panel_width      =   240;	// 実際に表示可能な幅
			cfg.panel_height     =   240;	// 実際に表示可能な高さ
			cfg.offset_x         =     0;	// パネルのX方向オフセット量
			cfg.offset_y         =     0;	// パネルのY方向オフセット量
			cfg.offset_rotation  =     0;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.dummy_read_pixel =     8;	// ピクセル読出し前のダミーリードのビット数
			cfg.dummy_read_bits  =     1;	// ピクセル以外のデータ読出し前のダミーリードのビット数
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.dlen_16bit       = false;	// データ長を16bit単位で送信するパネルの場合 trueに設定
			cfg.bus_shared       =  true;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

			_panel_instance.config(cfg);
		}
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。

			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

class LGFX_MSP2807 : public lgfx::LGFX_Device
{
	lgfx::Panel_ILI9341	_panel_instance;
	lgfx::Bus_SPI		_bus_instance;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
public:
	LGFX_MSP2807(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if defined(CONFIG_IDF_TARGET_ESP32)
		nvscfg_spi_t nvs = {
			.sclk = 25,
			.mosi = 26,
			.miso = 32,
			.dc   = 27,
			.cs   = 12,
			.rst  = 14,
			.busy = -1,
			.bl   = 33,
		};
	#elif defined(CONFIG_IDF_TARGET_ESP32S3)
		nvscfg_spi_t nvs = {
			.sclk = 9,
			.mosi = 10,
			.miso = 3,
			.dc   = 11,
			.cs   = 13,
			.rst  = 12,
			.busy = -1,
			.bl   = 46,
		};
	#elif defined(CONFIG_IDF_TARGET_ESP32C3)
		nvscfg_spi_t nvs = {
			.sclk = 5,
			.mosi = 6,
			.miso = 1,
			.dc   = 7,
			.cs   = 10,
			.rst  = 8,
			.busy = -1,
			.bl   = 4,
		};
	#else
		nvscfg_spi_t nvs = {0};
		return;
	#endif
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		Serial.printf("sclk=%d,mosi=%d,miso=%d,dc=%d,cs=%d,rst=%d,busy=%d,bl=%d\n",
					nvs.sclk, nvs.mosi, nvs.miso, nvs.dc, nvs.cs, nvs.rst, nvs.busy, nvs.bl);
		if(ChkFF(nvs.sclk)<0 || ChkFF(nvs.mosi)<0) return;

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
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_instance.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_instance);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			// ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。
			cfg.memory_width     =   240;	// ドライバICがサポートしている最大の幅
			cfg.memory_height    =   320;	// ドライバICがサポートしている最大の高さ
			cfg.panel_width      =   240;	// 実際に表示可能な幅
			cfg.panel_height     =   320;	// 実際に表示可能な高さ
			cfg.offset_x         =     0;	// パネルのX方向オフセット量
			cfg.offset_y         =     0;	// パネルのY方向オフセット量
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
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

			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

class LGFX_ATM0177B3A : public lgfx::LGFX_Device
{
	lgfx::Panel_ILI9163	_panel_instance;
	lgfx::Bus_SPI		_bus_instance;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
public:
	LGFX_ATM0177B3A(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if defined(CONFIG_IDF_TARGET_ESP32)
		nvscfg_spi_t nvs = {
			.sclk = 27,
			.mosi = 26,
			.miso = -1,
			.dc   = 14,
			.cs   = 13,
			.rst  = 12,
			.busy = -1,
			.bl   = -1,
		};
	#elif defined(CONFIG_IDF_TARGET_ESP32S3)
		nvscfg_spi_t nvs = {
			.sclk = 11,
			.mosi = 10,
			.miso = -1,
			.dc   = 12,
			.cs   = 8,
			.rst  = 13,
			.busy = -1,
			.bl   = -1,
		};
	#elif defined(CONFIG_IDF_TARGET_ESP32C3)
		nvscfg_spi_t nvs = {
			.sclk = 7,
			.mosi = 6,
			.miso = -1,
			.dc   = 8,
			.cs   = 0,
			.rst  = 10,
			.busy = -1,
			.bl   = -1,
		};
	#else
		nvscfg_spi_t nvs = {0};
		return;
	#endif
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		Serial.printf("sclk=%d,mosi=%d,miso=%d,dc=%d,cs=%d,rst=%d,busy=%d,bl=%d\n",
					nvs.sclk, nvs.mosi, nvs.miso, nvs.dc, nvs.cs, nvs.rst, nvs.busy, nvs.bl);
		if(ChkFF(nvs.sclk)<0 || ChkFF(nvs.mosi)<0) return;

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
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_instance.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_instance);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			// ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。
			cfg.memory_width     =   128;	// ドライバICがサポートしている最大の幅
			cfg.memory_height    =   160;	// ドライバICがサポートしている最大の高さ
			cfg.panel_width      =   128;	// 実際に表示可能な幅
			cfg.panel_height     =   160;	// 実際に表示可能な高さ
			cfg.offset_x         =     0;	// パネルのX方向オフセット量
			cfg.offset_y         =     0;	// パネルのY方向オフセット量
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.dummy_read_pixel =     8;	// ピクセル読出し前のダミーリードのビット数
			cfg.dummy_read_bits  =     1;	// ピクセル以外のデータ読出し前のダミーリードのビット数
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = false;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        =  true;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.dlen_16bit       = false;	// データ長を16bit単位で送信するパネルの場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

			_panel_instance.config(cfg);
		}
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。

			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

#if 0
class LGFX_SAINSMART18 : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7735S	_panel_instance;
	lgfx::Bus_SPI		_bus_instance;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
public:
	LGFX_SAINSMART18(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = {
			.sclk = 12,
			.mosi = 14,
			.miso = -1,
			.dc   = 27,
			.cs   = 25,
			.rst  = 26,
			.busy = -1,
			.bl   = -1,
		};

		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		Serial.printf("sclk=%d,mosi=%d,miso=%d,dc=%d,cs=%d,rst=%d,busy=%d,bl=%d\n",
					nvs.sclk, nvs.mosi, nvs.miso, nvs.dc, nvs.cs, nvs.rst, nvs.busy, nvs.bl);
		if(ChkFF(nvs.sclk)<0 || ChkFF(nvs.mosi)<0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_instance.config();	// バス設定用の構造体を取得します。

// SPIバスの設定
			cfg.spi_host = SPI2_HOST;			// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			// ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 4000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.spi_3wire  = false;				// 受信をMOSIピンで行う場合はtrueを設定
			cfg.use_lock   = true;				// トランザクションロックを使用する場合はtrueを設定
			cfg.dma_channel = SPI_DMA_CH_AUTO;	// 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
			// ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_instance.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_instance);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			// ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。
			cfg.memory_width     =   128;	// ドライバICがサポートしている最大の幅
			cfg.memory_height    =   160;	// ドライバICがサポートしている最大の高さ
			cfg.panel_width      =   128;	// 実際に表示可能な幅
			cfg.panel_height     =   160;	// 実際に表示可能な高さ
			cfg.offset_x         =     0;	// パネルのX方向オフセット量
			cfg.offset_y         =     0;	// パネルのY方向オフセット量
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.dummy_read_pixel =     8;	// ピクセル読出し前のダミーリードのビット数
			cfg.dummy_read_bits  =     1;	// ピクセル以外のデータ読出し前のダミーリードのビット数
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = false;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.dlen_16bit       = false;	// データ長を16bit単位で送信するパネルの場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

			_panel_instance.config(cfg);
		}
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};
#endif
