enum {
	LCDTYPE_NOLCD = 0,
	LCDTYPE_AUTO = 1,
	LCDTYPE_AUTO_ROT1 = 2,

	LCDTYPE_SSD1306 = 3,
	LCDTYPE_SSD1306_32 = 4,
	LCDTYPE_QT095B = 5,
	LCDTYPE_3248S035 = 6,
	LCDTYPE_ROUNDLCD = 7,
	LCDTYPE_MSP2806_2807 = 8,
	xLCDTYPE_ATM0177B3A = 9,

	// type1
	LCDTYPE_MSP0961 = 10,
	LCDTYPE_MSP1141 = 11,
	LCDTYPE_MSP1308_GMT130 = 12,
	LCDTYPE_MSP1541 = 13,
	LCDTYPE_GMT177 = 14,
	LCDTYPE_MSP2008 = 15,

	// type2
	LCDTYPE_MSP1443 = 16,
	LCDTYPE_MSP1803 = 17,
	LCDTYPE_MSP2202 = 18,
	LCDTYPE_MSP2401_2402 = 19,
	LCDTYPE_MSP3217_3218 = 20,
	LCDTYPE_MSP3520_3521 = 21,
	LCDTYPE_MSP4020_4021 = 22,
	LCDTYPE_MSP4022_4023 = 23,

	LCDTYPE_ROUNDXIAO = 24,
	LCDTYPE_SQUAREXIAO = 25,
	LCDTYPE_128TFT = 26,
	LCDTYPE_TTGO_TDISP = 27,

	LCDTYPE_SH110X = 28,
	LCDTYPE_1732S019 = 29,
	LCDTYPE_RP2040LCD128 = 30,
	LCDTYPE_085TFT_SPI = 31,
	LCDTYPE_SSD1306_6432 = 32,

	LCDTYPE_SQUARELCD = 33,
	LCDTYPE_RoundTouchXIAO = 34,
	LCDTYPE_RP2040GEEK = 35,
	LCDTYPE_ESP32C3_144 = 36,
};

const char LcdTypeStr[][16] = {
	"no-LCD",		// 0
	"AUTO",
	"AUTO_ROT1",
	"SSD1306",
	"SSD1306_32",
	"QT095B",
	"3248S035",
	"ROUNDLCD",
	"MSP2806_2807",
	"",

	"MSP0961",		// 10
	"MSP1141",
	"MSP1308_GMT130",
	"MSP1541",
	"GMT177",
	"MSP2008",

	"MSP1443",		// 16
	"MSP1803",
	"MSP2202",
	"MSP2401_2402",
	"MSP3217_3218",
	"MSP3520_3521",
	"MSP4020_4021",
	"MSP4022_4023",

	"ROUNDXIAO",	// 24
	"SQUAREXIAO",
	"128TFT",
	"TTGO_TDISP",

	"SH110X",		// 28
	"1732S019",
	"RP2040LCD128",
	"085TFT_SPI",
	"SSD1306_6432",

	"SQUARELCD",		// 33
	"RoundTouchXIAO",
	"RP2040GEEK",
	"ESP32C3_144",
};

typedef struct {
	uint8_t sda;
	uint8_t scl;
} nvscfg_i2c_t;

typedef struct {
	int8_t sclk;
	int8_t mosi;
	int8_t miso;
	int8_t dc;
	int8_t cs;
	int8_t rst;
	int8_t busy;
	int8_t bl;
} nvscfg_spi_t;

#define SUPPORT_XIAO
#if defined(CONFIG_IDF_TARGET_ESP32)
  #define PWM_CH  7
  int ChkFF(int a) {
    const uint8_t availables[] = {/*input*/34,35, 32,33,25,26,27,14,12, 13,15,2, 0,4,16,17,5,18,19,21,22,23};
    for(int i=0; i<sizeof(availables); i++)
      if(a == availables[i]) return a;
    return -1;
  }
  #undef SUPPORT_XIAO
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
  #define PWM_CH  3
  int ChkFF(int a) {
    const uint8_t availables[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17, /*usb*/18,19, /*uart*/20,21,};
    for(int i=0; i<sizeof(availables); i++)
      if(a == availables[i]) return a;
    return -1;
  }
  enum{ XIAO0= 2, XIAO1= 3, XIAO2= 4, XIAO3= 5, XIAO4=6, XIAO5=7, XIAO6=21, XIAO7=20, XIAO8=8, XIAO9=9, XIAO10=10,};
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
  #define PWM_CH  7
  int ChkFF(int a) {
    const uint8_t availables[] = {4,5,6,7,15,16,17,18,8,19,20, 3,46,9,10,11,12,13,14,21,47,48,45, 0,35,36,37,38,39,40,41,42,44,43,2,1};
    for(int i=0; i<sizeof(availables); i++)
      if(a == availables[i]) return a;
    return -1;
  }
  enum{ XIAO0= 1, XIAO1= 2, XIAO2= 3, XIAO3= 4, XIAO4=5, XIAO5=6, XIAO6=43, XIAO7=44, XIAO8=7, XIAO9=8, XIAO10=9, };
#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
  typedef int spi_host_device_t;
  #define PWM_CH  7
  int ChkFF(int a) {
    const uint8_t availables[] = {0,1,2,3,4,5,6,7,8,9, 10,11,12,13,14,15,16,17,18,19, 20,21,22,23,24,25,26,27,28,29};
    for(int i=0; i<sizeof(availables); i++)
      if(a == availables[i]) return a;
    return -1;
  }
  enum{ XIAO0=26, XIAO1=27, XIAO2=28, XIAO3=29, XIAO4=6, XIAO5=7, XIAO6= 0, XIAO7= 1, XIAO8=2, XIAO9=4, XIAO10=3, };
#else
  #error
#endif

spi_host_device_t getSpiCh(int sclk, int mosi)
{
#if defined(ESP32)
	if(ChkFF(sclk)<0 || ChkFF(mosi)<0) return (spi_host_device_t)-1;
	return SPI2_HOST;

#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
	int sclk_ch = -1;
	int mosi_ch = -1;

	switch(sclk) {
	case 2:
	case 6:
	case 18:
	case 22:
		sclk_ch = 0;
		break;
	case 10:
	case 14:
	case 26:
		sclk_ch = 1;
		break;
	}

	switch(mosi) {
	case 3:
	case 7:
	case 19:
	case 23:
		mosi_ch = 0;
		break;
	case 11:
	case 15:
	case 27:
		mosi_ch = 1;
		break;
	}

	if(sclk_ch == mosi_ch)
		return (spi_host_device_t)sclk_ch;
	return (spi_host_device_t)-1;
#else
	return (spi_host_device_t)-1;
#endif
}

void setPortHi(int port)
{
	if(ChkFF(port) < 0) return;
	pinMode(port, OUTPUT);
	digitalWrite(port, 1);
}

#if defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
lgfx::Bus_SPI		_bus_spi;
#endif

///////////////////////////////////////////////////////////////
// I2C monochrome

#if defined(ESP32)
class LGFX_SSD1306 : public lgfx::LGFX_Device
{
	lgfx::Panel_SSD1306	_panel_instance;
	lgfx::Bus_I2C		_bus_i2c;
public:
	LGFX_SSD1306(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if defined(CONFIG_IDF_TARGET_ESP32)
		nvscfg_i2c_t nvs = { .sda=21, .scl=22, };
	#elif defined(CONFIG_IDF_TARGET_ESP32S3)
		nvscfg_i2c_t nvs = { .sda=8,  .scl=9, };
	#elif defined(CONFIG_IDF_TARGET_ESP32C3)
		nvscfg_i2c_t nvs = { .sda=6,  .scl=7, };
	#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
		nvscfg_i2c_t nvs = { .sda=4,  .scl=5, };
	#else
		nvscfg_i2c_t nvs = {0};
		return;
	#endif
		if(config_size >= sizeof(nvscfg_i2c_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		if(ChkFF(nvs.sda)<0 || ChkFF(nvs.scl)<0) return;

		{ // バス制御の設定
			auto cfg = _bus_i2c.config();
			cfg.i2c_port	= 0;				// 使用するI2Cポートを選択 (0 or 1)
			cfg.freq_write	= 400000;			// 送信時のクロック
			cfg.freq_read	= 400000;			// 受信時のクロック
			cfg.pin_sda		= ChkFF(nvs.sda);	// SDAを接続しているピン番号
			cfg.pin_scl		= ChkFF(nvs.scl);	// SCLを接続しているピン番号
			cfg.i2c_addr	= 0x3C;				// I2Cデバイスのアドレス
			_bus_i2c.config(cfg);				// 設定値をバスに反映
			_panel_instance.setBus(&_bus_i2c);
		}

		{ // 表示パネル制御の設定
			auto cfg = _panel_instance.config();
			switch(lcdType) {
			case LCDTYPE_SSD1306:
				cfg.panel_width		= 128;		// 実際に表示可能な幅
				cfg.panel_height	= 64;		// 実際に表示可能な高さ
				break;
			case LCDTYPE_SSD1306_32:
				cfg.panel_width		= 128;		// 実際に表示可能な幅
				cfg.panel_height	= 32;		// 実際に表示可能な高さ
				_panel_instance.setComPins(0x02);	// 0x12 for 128x64, 0x02 for 128x32
				break;
			case LCDTYPE_SSD1306_6432:
				cfg.panel_width		= 64;		// 実際に表示可能な幅
				cfg.panel_height	= 32;		// 実際に表示可能な高さ
				cfg.offset_x        = 32;
				break;
			}
			_panel_instance.config(cfg);
		}
		setPanel(&_panel_instance);	// 使用するパネルをセット
		init();
	}
};

class LGFX_SH110X : public lgfx::LGFX_Device
{
	lgfx::Panel_SH110x	_panel_instance;
	lgfx::Bus_I2C		_bus_i2c;
public:
	LGFX_SH110X(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if defined(CONFIG_IDF_TARGET_ESP32)
		nvscfg_i2c_t nvs = { .sda=21, .scl=22, };
	#elif defined(CONFIG_IDF_TARGET_ESP32S3)
		nvscfg_i2c_t nvs = { .sda=8,  .scl=9, };
	#elif defined(CONFIG_IDF_TARGET_ESP32C3)
		nvscfg_i2c_t nvs = { .sda=6,  .scl=7, };
	#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
		nvscfg_i2c_t nvs = { .sda=4,  .scl=5, };
	#else
		nvscfg_i2c_t nvs = {0};
		return;
	#endif
		if(config_size >= sizeof(nvscfg_i2c_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		if(ChkFF(nvs.sda)<0 || ChkFF(nvs.scl)<0) return;

		{ // バス制御の設定
			auto cfg = _bus_i2c.config();
			cfg.i2c_port	= 0;				// 使用するI2Cポートを選択 (0 or 1)
			cfg.freq_write	= 400000;			// 送信時のクロック
			cfg.freq_read	= 400000;			// 受信時のクロック
			cfg.pin_sda		= ChkFF(nvs.sda);	// SDAを接続しているピン番号
			cfg.pin_scl		= ChkFF(nvs.scl);	// SCLを接続しているピン番号
			cfg.i2c_addr	= 0x3C;				// I2Cデバイスのアドレス
			_bus_i2c.config(cfg);			// 設定値をバスに反映
			_panel_instance.setBus(&_bus_i2c);
		}

		{ // 表示パネル制御の設定
			auto cfg = _panel_instance.config();
			cfg.panel_width		= 128;		// 実際に表示可能な幅
			cfg.panel_height	= 128;
											// 実際に表示可能な高さ
			_panel_instance.config(cfg);
		}
		setPanel(&_panel_instance);	// 使用するパネルをセット
		init();
	}
};
#endif // ESP32

///////////////////////////////////////////////////////////////
// Type1  GND/3.3V/CLK/MOSI/RST/DC/CS/BLK

#if defined(CONFIG_IDF_TARGET_ESP32)
	const nvscfg_spi_t nvsType1 = { .sclk=33, .mosi=25, .miso=-1, .dc=27, .cs=14, .rst=26, .busy=-1, .bl=12, };
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
	const nvscfg_spi_t nvsType1 = { .sclk=46, .mosi= 9, .miso=-1, .dc=11, .cs=12, .rst=10, .busy=-1, .bl=13, };
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
	const nvscfg_spi_t nvsType1 = { .sclk= 4, .mosi= 5, .miso=-1, .dc= 7, .cs= 8, .rst= 6, .busy=-1, .bl=-1, };
#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
	const nvscfg_spi_t nvsType1 = { .sclk=10, .mosi=11, .miso=-1, .dc=13, .cs=14, .rst=12, .busy=-1, .bl=15, };
#else
	const nvscfg_spi_t nvsType1 = {-1,-1,-1,-1,-1,-1,-1,-1,};
#endif

class LGFX_085TFT_SPI : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7735S	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_085TFT_SPI(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = {0};
		memcpy(&nvs, &nvsType1, sizeof(nvs));
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   128;	// 実際に表示可能な幅
			cfg.panel_height     =   128;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     0;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.offset_x         =     2;
			cfg.offset_y         =     1;
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

class LGFX_QT095B : public lgfx::LGFX_Device
{
	lgfx::Panel_SSD1331	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
#endif
public:
	LGFX_QT095B(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = {0};
		memcpy(&nvs, &nvsType1, sizeof(nvs));
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =    96;	// 実際に表示可能な幅
			cfg.panel_height     =    64;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     0;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = false;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        =  true;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

class LGFX_MSP0961 : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7735S	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_MSP0961(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = {0};
		memcpy(&nvs, &nvsType1, sizeof(nvs));
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =    80;	// 実際に表示可能な幅
			cfg.panel_height     =   160;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.offset_x         =    26;
			cfg.offset_y         =     1;
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

class LGFX_MSP1141 : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7789	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_MSP1141(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = {0};
		memcpy(&nvs, &nvsType1, sizeof(nvs));
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   135;	// 実際に表示可能な幅
			cfg.panel_height     =   240;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

// MSP1308_GMT130/GMT130	MSP1541
class LGFX_MSP1308_GMT130 : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7789	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_MSP1308_GMT130(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if defined(CONFIG_IDF_TARGET_ESP32)
		nvscfg_spi_t nvs = { .sclk=33, .mosi=25, .miso=-1, .dc=27, .cs=-1, .rst=26, .busy=-1, .bl=14, };
	#elif defined(CONFIG_IDF_TARGET_ESP32S3)
		nvscfg_spi_t nvs = { .sclk=46, .mosi= 9, .miso=-1, .dc=11, .cs=-1, .rst=10, .busy=-1, .bl=12, };
	#elif defined(CONFIG_IDF_TARGET_ESP32C3)
		nvscfg_spi_t nvs = { .sclk= 4, .mosi= 5, .miso=-1, .dc= 7, .cs=-1, .rst= 6, .busy=-1, .bl= 8, };
	#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
		nvscfg_spi_t nvs = { .sclk=10, .mosi=11, .miso=-1, .dc=13, .cs=-1, .rst=12, .busy=-1, .bl=14, };
	#else
		nvscfg_spi_t nvs = {0};
		return;
	#endif
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 3;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   240;	// 実際に表示可能な幅
			cfg.panel_height     =   240;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     0;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

// MSP1541
class LGFX_MSP1541 : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7789	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_MSP1541(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = {0};
		memcpy(&nvs, &nvsType1, sizeof(nvs));
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   240;	// 実際に表示可能な幅
			cfg.panel_height     =   240;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     0;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

class LGFX_GMT177 : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7735S	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_GMT177(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = {0};
		memcpy(&nvs, &nvsType1, sizeof(nvs));
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   128;	// 実際に表示可能な幅
			cfg.panel_height     =   160;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.offset_x         =     4;
			cfg.offset_y         =     0;
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = false;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        =  true;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

class LGFX_MSP2008 : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7789	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_MSP2008(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = {0};
		memcpy(&nvs, &nvsType1, sizeof(nvs));
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   320;	// 実際に表示可能な幅
			cfg.panel_height     =   240;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     0;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

class LGFX_128TFT : public lgfx::LGFX_Device
{
	lgfx::Panel_GC9A01	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_128TFT(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if defined(CONFIG_IDF_TARGET_ESP32)
		nvscfg_spi_t nvs = { .sclk=33, .mosi=25, .miso=-1, .dc=26, .cs=27, .rst=14, .busy=-1, .bl=-1, };
	#elif defined(CONFIG_IDF_TARGET_ESP32S3)
		nvscfg_spi_t nvs = { .sclk=46, .mosi= 9, .miso=-1, .dc=10, .cs=11, .rst=12, .busy=-1, .bl=-1, };
	#elif defined(CONFIG_IDF_TARGET_ESP32C3)
		nvscfg_spi_t nvs = { .sclk= 4, .mosi= 5, .miso=-1, .dc= 7, .cs= 8, .rst= 6, .busy=-1, .bl=-1, };
	#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
		nvscfg_spi_t nvs = { .sclk=10, .mosi=11, .miso=-1, .dc=12, .cs=13, .rst=14, .busy=-1, .bl=-1, };
	#else
		nvscfg_spi_t nvs = {0};
		return;
	#endif
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   240;	// 実際に表示可能な幅
			cfg.panel_height     =   240;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     0;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       =  true;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

///////////////////////////////////////////////////////////////
// Type2  (MISO)/BL/CLK/MOSI/DC/RST/CS/GND/5V

#if defined(CONFIG_IDF_TARGET_ESP32)
	const nvscfg_spi_t nvsType2 = { .sclk=25, .mosi=26, .miso=32, .dc=27, .cs=12, .rst=14, .busy=-1, .bl=33, };
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
	const nvscfg_spi_t nvsType2 = { .sclk= 9, .mosi=10, .miso= 3, .dc=11, .cs=13, .rst=12, .busy=-1, .bl=46, };
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
	const nvscfg_spi_t nvsType2 = { .sclk= 5, .mosi= 6, .miso= 1, .dc= 7, .cs=10, .rst= 8, .busy=-1, .bl= 4, };
#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
	const nvscfg_spi_t nvsType2 = { .sclk=10, .mosi=11, .miso= 8, .dc=12, .cs=14, .rst=13, .busy=-1, .bl= 9, };
#else
	const nvscfg_spi_t nvsType2 = {-1,-1,-1,-1,-1,-1,-1,-1,};
#endif

class LGFX_MSP1443 : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7735S	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_MSP1443(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = {0};
		memcpy(&nvs, &nvsType2, sizeof(nvs));
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
		#if defined(USE_SD)
			cfg.spi_3wire  = true;				// 受信をMOSIピンで行う場合はtrueを設定
		#endif
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   128;	// 実際に表示可能な幅
			cfg.panel_height     =   128;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     0;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.offset_x         =     2;
			cfg.offset_y         =     1;
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = false;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

class LGFX_MSP1803 : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7735S	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_MSP1803(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = {0};
		memcpy(&nvs, &nvsType2, sizeof(nvs));
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   128;	// 実際に表示可能な幅
			cfg.panel_height     =   160;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.offset_x         =     4;
			cfg.offset_y         =     0;
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = false;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        =  true;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

// MSP2202/MSP2401/MSP2402/MSP2806/MSP2807/MSP3217/MSP3218
class LGFX_MSP2202_240x_280x_321x : public lgfx::LGFX_Device
{
	lgfx::Panel_ILI9341	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_MSP2202_240x_280x_321x(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = {0};
		memcpy(&nvs, &nvsType2, sizeof(nvs));
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   240;	// 実際に表示可能な幅
			cfg.panel_height     =   320;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = false;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

// MSP3520/MSP3521
class LGFX_MSP352x : public lgfx::LGFX_Device
{
	lgfx::Panel_ILI9488	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_MSP352x(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = {0};
		memcpy(&nvs, &nvsType2, sizeof(nvs));
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   480;	// 実際に表示可能な幅
			cfg.panel_height     =   320;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = false;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

// MSP4020/MSP4021
class LGFX_MSP4020_4021 : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7796	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_MSP4020_4021(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = {0};
		memcpy(&nvs, &nvsType2, sizeof(nvs));
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   480;	// 実際に表示可能な幅
			cfg.panel_height     =   320;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = false;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

// MSP4022/MSP4023
class LGFX_MSP4022_4023 : public lgfx::LGFX_Device
{
	lgfx::Panel_ILI9486	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_MSP4022_4023(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = {0};
		memcpy(&nvs, &nvsType2, sizeof(nvs));
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   480;	// 実際に表示可能な幅
			cfg.panel_height     =   320;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = false;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

///////////////////////////////////////////////////////////////
// XIAO

class LGFX_ROUNDXIAO : public lgfx::LGFX_Device
{
	lgfx::Panel_GC9A01	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_ROUNDXIAO(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if defined(SUPPORT_XIAO)
		nvscfg_spi_t nvs = { .sclk=XIAO8, .mosi=XIAO10, .miso=XIAO9, .dc=XIAO0, .cs=XIAO7, .rst=XIAO1, .busy=-1, .bl=XIAO2, };
		#if defined(USE_SD)
		pin_sdcs = 21;
		#endif
	#else
		nvscfg_spi_t nvs = {0};
		return;
	#endif
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
		#if defined(USE_SD)
			cfg.spi_3wire  = true;				// 受信をMOSIピンで行う場合はtrueを設定
		#endif
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   240;	// 実際に表示可能な幅
			cfg.panel_height     =   240;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     0;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       =  true;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

class LGFX_SQUAREXIAO : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7789	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_SQUAREXIAO(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if defined(SUPPORT_XIAO)
		nvscfg_spi_t nvs = { .sclk=XIAO8, .mosi=XIAO10, .miso=XIAO9, .dc=XIAO0, .cs=XIAO7, .rst=XIAO1, .busy=-1, .bl=XIAO2, };
		#if defined(USE_SD)
		pin_sdcs = 21;
		#endif
	#else
		nvscfg_spi_t nvs = {0};
		return;
	#endif
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
		#if defined(USE_SD)
			cfg.spi_3wire  = true;				// 受信をMOSIピンで行う場合はtrueを設定
		#endif
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   240;	// 実際に表示可能な幅
			cfg.panel_height     =   280;	// 実際に表示可能な高さ
		#if defined(CONFIG_IDF_TARGET_ESP32S3)
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
		#else
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
		#endif
			cfg.offset_x         =     0;
			cfg.offset_y         =    20;
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       =  true;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

class LGFX_RoundTouchXIAO : public lgfx::LGFX_Device
{
	lgfx::Panel_GC9A01	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_RoundTouchXIAO(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if defined(SUPPORT_XIAO)
		nvscfg_spi_t nvs = { .sclk=XIAO8, .mosi=XIAO10, .miso=XIAO9, .dc=XIAO3, .cs=XIAO1, .rst=-1, .busy=-1, .bl=XIAO6, };
		#if defined(USE_SD)
		pin_sdcs = 21;//XIAO2;
		#endif
	#else
		nvscfg_spi_t nvs = {0};
		return;
	#endif
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
		#if defined(USE_SD)
			cfg.spi_3wire  = true;				// 受信をMOSIピンで行う場合はtrueを設定
		#endif
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   240;	// 実際に表示可能な幅
			cfg.panel_height     =   240;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     3;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       =  true;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

///////////////////////////////////////////////////////////////
// other

class LGFX_ROUNDLCD : public lgfx::LGFX_Device
{
	lgfx::Panel_GC9A01	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_ROUNDLCD(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if defined(CONFIG_IDF_TARGET_ESP32)
		nvscfg_spi_t nvs = { .sclk=14, .mosi=12, .miso=-1, .dc=16, .cs=17, .rst=15, .busy=-1, .bl=13, };
	#elif defined(CONFIG_IDF_TARGET_ESP32S3)
	  #if defined(CAMERA_ENABLED)
		nvscfg_spi_t nvs = { .sclk=21, .mosi=47, .miso=-1, .dc=40, .cs=14, .rst=39, .busy=-1, .bl=48, };
	  #else
		nvscfg_spi_t nvs = { .sclk= 1, .mosi=15, .miso=43, .dc= 7, .cs= 5, .rst=13, .busy=-1, .bl= 9, };	// SDCS=44
		#if defined(USE_SD)
		pin_sdcs = 44;
		#endif
	  #endif
  	#elif defined(CONFIG_IDF_TARGET_ESP32C3)
		nvscfg_spi_t nvs = { .sclk= 4, .mosi= 6, .miso=-1, .dc= 1, .cs= 7, .rst= 0, .busy=-1, .bl=10, };
	#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
		nvscfg_spi_t nvs = { .sclk=18, .mosi=19, .miso=16, .dc=11, .cs=13, .rst=12, .busy=-1, .bl=10, };	// SDCS=16
		#if defined(USE_SD)
		pin_sdcs = 16;
		#endif
	#else
		nvscfg_spi_t nvs = {0};
		return;
	#endif
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
		#if defined(USE_SD)
			cfg.spi_3wire  = true;				// 受信をMOSIピンで行う場合はtrueを設定
		#endif
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   240;	// 実際に表示可能な幅
			cfg.panel_height     =   240;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     0;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       =  true;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

class LGFX_SQUARELCD : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7789	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_SQUARELCD(int lcdType, uint8_t *config_buf, int config_size)
	{
	#if defined(CONFIG_IDF_TARGET_ESP32)
		nvscfg_spi_t nvs = {0};
		return;
	#elif defined(CONFIG_IDF_TARGET_ESP32S3)
	  #if defined(CAMERA_ENABLED)
		nvscfg_spi_t nvs = { .sclk=21, .mosi=47, .miso=-1, .dc=40, .cs=14, .rst=39, .busy=-1, .bl=48, };	// squareCam
	  #else
		nvscfg_spi_t nvs = { .sclk= 1, .mosi=15, .miso=43, .dc= 7, .cs= 5, .rst=13, .busy=-1, .bl= 9, };	// SDCS=44
		#if defined(USE_SD)
		pin_sdcs = 44;
		#endif
	  #endif
  	#elif defined(CONFIG_IDF_TARGET_ESP32C3)
		nvscfg_spi_t nvs = {0};
		return;
	#elif defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
		nvscfg_spi_t nvs = {0};
		return;
	#else
		nvscfg_spi_t nvs = {0};
		return;
	#endif
		if(config_size >= sizeof(nvscfg_spi_t))
			memcpy(&nvs, config_buf, sizeof(nvs));

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
		#if defined(USE_SD)
			cfg.spi_3wire  = true;				// 受信をMOSIピンで行う場合はtrueを設定
		#endif
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   240;	// 実際に表示可能な幅
			cfg.panel_height     =   280;	// 実際に表示可能な高さ
		#if defined(CONFIG_IDF_TARGET_ESP32S3)
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
		#else
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
		#endif
			cfg.offset_x         =     0;
			cfg.offset_y         =    20;
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       =  true;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

#if defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
// SDA	6
// SCL	7

class LGFX_RP2040LCD128 : public lgfx::LGFX_Device
{
	lgfx::Panel_GC9A01	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_RP2040LCD128(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = { .sclk=10, .mosi=11, .miso=-1, .dc= 8, .cs= 9, .rst=12, .busy=-1, .bl=25, };

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   240;	// 実際に表示可能な幅
			cfg.panel_height     =   240;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     0;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       =  true;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};

// RP2040-GEEK
class LGFX_RP2040GEEK : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7789	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_RP2040GEEK(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = { .sclk=10, .mosi=11, .miso=-1, .dc= 8, .cs= 9, .rst=12, .busy=-1, .bl=25, };

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   135;	// 実際に表示可能な幅
			cfg.panel_height     =   240;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.offset_x         =    52;
			cfg.offset_y         =    40;
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 1200;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
		setTextSize(2);
	}
};

#endif	// RP2040

///////////////////////////////////////////////////////////////
// micom+LCD

#if defined(CONFIG_IDF_TARGET_ESP32)
// 3248S035
class LGFX_3248S035 : public lgfx::LGFX_Device
{
	lgfx::Panel_GC9A01	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_3248S035(int lcdType, uint8_t *config_buf, int config_size)
	{
		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = SPI2_HOST;			// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = 14;					// SPIのSCLKピン番号を設定
			cfg.pin_mosi = 13;					// SPIのMOSIピン番号を設定
			cfg.pin_miso = 12;					// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   =  2;					// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs           =    15;	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst          =    -1;	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy         =    -1;	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   320;	// 実際に表示可能な幅
			cfg.panel_height     =   480;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     5;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = false;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = 27;				// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
		setTextSize(2);
		#if defined(USE_SD)
		pin_sdcs = 5;
		pin_sdclk = 18;
		pin_sdmosi = 23;
		pin_sdmiso = 19;
		#endif
	}
};

// TTGO T-DISP
class LGFX_TTGO_TDISP : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7789	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_TTGO_TDISP(int lcdType, uint8_t *config_buf, int config_size)
	{
		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = SPI2_HOST;			// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = 18;					// SPIのSCLKピン番号を設定
			cfg.pin_mosi = 19;					// SPIのMOSIピン番号を設定
			cfg.pin_miso = -1;					// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = 16;					// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs           =     5;	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst          =    23;	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy         =    -1;	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   135;	// 実際に表示可能な幅
			cfg.panel_height     =   240;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.offset_x         =    52;
			cfg.offset_y         =    40;
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = 4;					// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 1200;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
		setTextSize(2);
	}
};
#endif	// TARGET_ESP32

#if defined(CONFIG_IDF_TARGET_ESP32S3)
// 1732S019
class LGFX_1732S019 : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7789	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_1732S019(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = { .sclk=12, .mosi=13, .miso=-1, .dc=11, .cs=10, .rst= 1, .busy=-1, .bl=14, };

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   170;	// 実際に表示可能な幅
			cfg.panel_height     =   320;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     1;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.offset_x         =    35;
			cfg.offset_y         =     0;
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           =  true;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
		setTextSize(2);
	}
};
#endif	// TARGET_ESP32S3


#if defined(CONFIG_IDF_TARGET_ESP32C3)
class LGFX_ESP32C3_144 : public lgfx::LGFX_Device
{
	lgfx::Panel_ST7735S	_panel_instance;
#if defined(ESP32)
	lgfx::Bus_SPI		_bus_spi;			// SPIバスのインスタンス
	lgfx::Light_PWM		_light_instance;
#endif
public:
	LGFX_ESP32C3_144(int lcdType, uint8_t *config_buf, int config_size)
	{
		nvscfg_spi_t nvs = { .sclk=3, .mosi=4, .miso=-1, .dc=0, .cs=2, .rst= 5, .busy=-1, .bl=-1, };

		spi_host_device_t spi_ch = getSpiCh(nvs.sclk, nvs.mosi);
		if(spi_ch < 0) return;

		{ // バス制御の設定を行います。
			auto cfg = _bus_spi.config();	// バス設定用の構造体を取得します。
			cfg.spi_host = spi_ch;				// 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
			cfg.spi_mode = 0;					// SPI通信モードを設定 (0 ~ 3)
			cfg.freq_write = 40000000;			// 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
			cfg.freq_read  = 16000000;			// 受信時のSPIクロック
			cfg.pin_sclk = ChkFF(nvs.sclk);		// SPIのSCLKピン番号を設定
			cfg.pin_mosi = ChkFF(nvs.mosi);		// SPIのMOSIピン番号を設定
			cfg.pin_miso = ChkFF(nvs.miso);		// SPIのMISOピン番号を設定 (-1 = disable)
			cfg.pin_dc   = ChkFF(nvs.dc);		// SPIのD/Cピン番号を設定  (-1 = disable)
		 // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
			_bus_spi.config(cfg);			// 設定値をバスに反映します。
			_panel_instance.setBus(&_bus_spi);		// バスをパネルにセットします。
		}

		{ // 表示パネル制御の設定を行います。
			auto cfg = _panel_instance.config();	// 表示パネル設定用の構造体を取得します。
			cfg.pin_cs   = ChkFF(nvs.cs);	// CSが接続されているピン番号   (-1 = disable)
			cfg.pin_rst  = ChkFF(nvs.rst);	// RSTが接続されているピン番号  (-1 = disable)
			cfg.pin_busy = ChkFF(nvs.busy);	// BUSYが接続されているピン番号 (-1 = disable)
			cfg.panel_width      =   128;	// 実際に表示可能な幅
			cfg.panel_height     =   128;	// 実際に表示可能な高さ
			cfg.offset_rotation  =     2;	// 回転方向の値のオフセット 0~7 (4~7は上下反転)
			cfg.offset_x         =     2;
			cfg.offset_y         =    31;
			cfg.readable         = false;	// データ読出しが可能な場合 trueに設定
			cfg.invert           = false;	// パネルの明暗が反転してしまう場合 trueに設定
			cfg.rgb_order        = false;	// パネルの赤と青が入れ替わってしまう場合 trueに設定
			cfg.bus_shared       = false;	// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
			_panel_instance.config(cfg);
		}
	#if defined(ESP32)
		{ // バックライト制御の設定を行います。（必要なければ削除）
			auto cfg = _light_instance.config();	// バックライト設定用の構造体を取得します。
			cfg.pin_bl = ChkFF(nvs.bl);		// バックライトが接続されているピン番号
			cfg.invert = false;				// バックライトの輝度を反転させる場合 true
			cfg.freq   = 44100;				// バックライトのPWM周波数
			cfg.pwm_channel = PWM_CH;		// 使用するPWMのチャンネル番号
			_light_instance.config(cfg);
			_panel_instance.setLight(&_light_instance);	// バックライトをパネルにセットします。
		}
	#else
		setPortHi(nvs.bl);
	#endif
		setPanel(&_panel_instance); // 使用するパネルをセットします。
		init();
	}
};
#endif	// TARGET_ESP32C3

