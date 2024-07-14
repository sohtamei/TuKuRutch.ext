/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include "main.h"

#include <TukurutchEspCamera2.h>
#include <TukurutchEspCamera2.cpp.h>

WebsocketsServer wsServer;

#define numof(a) (sizeof(a)/sizeof((a)[0]))

static int board_id = 0;

struct { m5::board_t board; char name[32]; } const boardTable[] = {
	{ m5::board_t::board_unknown,		"unknown" },		// 0
	{ m5::board_t::board_M5Stack,		"Stack" },			// 1
	{ m5::board_t::board_M5StackCore2,	"StackCore2" },		// 2
	{ m5::board_t::board_M5StickC,		"StickC" },			// 3
	{ m5::board_t::board_M5StickCPlus,	"StickCPlus" },		// 4

	{ m5::board_t::board_M5StackCoreInk,"CoreInk" },		// 5
	{ m5::board_t::board_M5Paper,		"Paper" },			// 6
	{ m5::board_t::board_M5Tough,		"Tough" },			// 7
	{ m5::board_t::board_M5Station,		"Station" },		// 8
	{ m5::board_t::board_M5StackCoreS3,	"StackCoreS3" },	// 9

	{ m5::board_t::board_M5AtomS3,		"ATOM S3" },		// 10
	{ m5::board_t::board_M5Atom,		"ATOM" },			// 11
	{ m5::board_t::board_M5AtomPsram,	"ATOM PSRAM" },		// 12
	{ m5::board_t::board_M5AtomU,		"ATOM U" },			// 13
	{ m5::board_t::board_M5StampPico,	"StampPico" },		// 14

	{ m5::board_t::board_M5StampC3,		"StampC3" },		// 15
	{ m5::board_t::board_M5StampC3U,	"StampC3U" },		// 16
	{ m5::board_t::board_M5AtomS3Lite,	"M5AtomS3Lite" },	// 17
};

/*
	int battery = M5.Power.getBatteryLevel();
	M5.Display.fillRect(ox, h * (i+2), px, h, M5.Display.getBaseColor());
*/

uint8_t _getSw(uint8_t button)
{
	switch(button) {
	case 0: return M5.BtnA.isPressed();
	case 1: return M5.BtnB.isPressed();
	case 2: return M5.BtnC.isPressed();
	case 3: return M5.BtnPWR.isPressed();
	}
	return 0;
/*
	/// BtnA,BtnB,BtnC,BtnEXT: "isPressed"/"wasPressed"/"isReleased"/"wasReleased"/"wasClicked"/"wasHold"/"isHolding"  can be use.
	int state = false;
	switch(button) {
	case 0: //return M5.BtnA.isPressed();
		state = M5.BtnA.wasHold() ? 1
			  : M5.BtnA.wasClicked() ? 2
			  : M5.BtnA.wasPressed() ? 3
			  : M5.BtnA.wasReleased() ? 4
			  : M5.BtnA.wasDeciedClickCount() ? 5
			  : 0;
		break;
	case 1: //return M5.BtnB.isPressed();
		state = M5.BtnB.wasHold() ? 1
			  : M5.BtnB.wasClicked() ? 2
			  : M5.BtnB.wasPressed() ? 3
			  : M5.BtnB.wasReleased() ? 4
			  : M5.BtnB.wasDeciedClickCount() ? 5
			  : 0;
		break;
	case 2: //return M5.BtnC.isPressed();
		state = M5.BtnC.wasHold() ? 1
			  : M5.BtnC.wasClicked() ? 2
			  : M5.BtnC.wasPressed() ? 3
			  : M5.BtnC.wasReleased() ? 4
			  : M5.BtnC.wasDeciedClickCount() ? 5
			  : 0;
		break;
	case 3: //return M5.BtnPWR.isPressed();
		state = M5.BtnPWR.wasHold() ? 1
			  : M5.BtnPWR.wasClicked() ? 2
			  : M5.BtnPWR.wasPressed() ? 3
			  : M5.BtnPWR.wasReleased() ? 4
			  : M5.BtnPWR.wasDeciedClickCount() ? 5
			  : 0;
		break;
	}
	return state;
*/
}

float getIMU(uint8_t index)
{
	if(!M5.Imu.isEnabled()) return 0;

	float data[3] = {0};
	if(index < 3) {
		M5.Imu.getAccel(data+0,data+1,data+2);
		return data[index-0];
	} else if(index < 6) {
		M5.Imu.getGyro(data+0,data+1,data+2);
		return data[index-3];
	} else if(index < 9) {
		//M5.Imu.getAhrsData(data+0,data+1,data+2);
		return data[index-6];
	} else {
	//	M5.Imu.getTemp(data+0);
		return data[0];
	}
}

int getIMUs(uint8_t index, uint8_t* buf)
{
	if(!M5.Imu.isEnabled()) return 0;

	float data[3] = {0};
	if(index < 3) {
		M5.Imu.getAccel(data+0,data+1,data+2);
	} else if(index < 6) {
		M5.Imu.getGyro(data+0,data+1,data+2);
	} else {
		return 0;
	}
	memcpy(buf, data, sizeof(data));
	return sizeof(data);
}

void __tone(int16_t freq, int16_t ms)
{
	if(!M5.Speaker.isEnabled()) return;
	M5.Speaker.tone(freq, ms);
	while(M5.Speaker.isPlaying()) { vTaskDelay(1); }
}

int _getConfig(uint8_t* buf)
{
	int enabled =
		  (M5.Display.getBoard() > m5gfx::boards::board_t::board_unknown ? (1<<0) : 0)
		| (M5.In_I2C.isEnabled()  ? (1<<1) : 0)
		| (M5.Ex_I2C.isEnabled()  ? (1<<2) : 0)
		| (M5.Imu.isEnabled()     ? (1<<3) : 0)
		| (M5.Speaker.isEnabled() ? (1<<4) : 0)
		| (M5.Mic.isEnabled()     ? (1<<5) : 0)
		| (M5.Touch.isEnabled()   ? (1<<6) : 0)
		| (M5.Rtc.isEnabled()     ? (1<<7) : 0)
	#if defined (CONFIG_IDF_TARGET_ESP32)
		| (M5.Power.Axp192.isEnabled()  ? (1<<8) : 0)
		| (M5.Power.Ip5306.isEnabled()  ? (1<<9) : 0)
	#elif defined (CONFIG_IDF_TARGET_ESP32S3)
		| (M5.Power.Axp2101.isEnabled() ? (1<<10) : 0)
	#endif
		;

	SetL16(buf+0, M5.Display.width());
	SetL16(buf+2, M5.Display.height());
	SetL16(buf+4, board_id);
	SetL16(buf+6, enabled);
	buf[8] = M5.Ex_I2C.getSDA();
	buf[9] = M5.Ex_I2C.getSCL();
	return 10;
}

static void onConnect(String ip)
{
//	M5.Display.fillScreen(TFT_BLACK);
//	M5.Display.setCursor(0,0);
	M5.Display.println(ip);
	wsServer.listen(PORT_WEBSOCKET);
//	M5.Power.setLed(255);
	espcamera_onConnect();
	Serial.println(ip);
}

void _setup(const char* ver)
{
	auto cfg = M5.config();
	cfg.serial_baudrate = 115200;	// default=115200. if "Serial" is not needed, set it to 0.
	cfg.clear_display = true;		// default=true. clear the screen when begin.
	cfg.output_power  = true;		// default=true. use external port 5V output.
	cfg.internal_imu  = true;		// default=true. use internal IMU.
	cfg.internal_rtc  = false;		// default=true. use internal RTC.
	cfg.internal_spk  = true;		// default=true. use internal speaker.
	cfg.internal_mic  = false;		// default=true. use internal microphone.
	cfg.led_brightness = 0;			// default= 0. system LED brightness (0=off / 255=max) (※ not NeoPixel)
	M5.begin(cfg);

	if(M5.Speaker.isEnabled()) {
		M5.Speaker.setVolume(128);
		M5.Speaker.tone(2000/*Hz*/, 100/*ms*/);
		while(M5.Speaker.isPlaying()) { vTaskDelay(1); }
	}

	/// For models with EPD : refresh control
	M5.Display.setEpdMode(epd_mode_t::epd_fastest); // fastest but very-low quality.
//	M5.Display.setEpdMode(epd_mode_t::epd_fast   ); // fast but low quality.
//	M5.Display.setEpdMode(epd_mode_t::epd_text   ); // slow but high quality. (for text)
//	M5.Display.setEpdMode(epd_mode_t::epd_quality); // slow but high quality. (for image)

	M5.Display.setBrightness(128/*0~255*/);

	if(M5.Display.width() < M5.Display.height()) {
		/// Landscape mode.
		M5.Display.setRotation(M5.Display.getRotation() ^ 1);
	}

	M5.Display.fillScreen(TFT_BLACK);
	M5.Display.setCursor(0, 0);
	int textsize = M5.Display.height() / 160;
	if(textsize < 2) { textsize = 2; }
	M5.Display.setTextSize(textsize);

	M5.Display.println(ver);

	const char* name = "unknown";
	int i;
	for(i = 0; i < sizeof(boardTable)/sizeof(boardTable[0]); i++) {
		if(M5.getBoard() == boardTable[i].board) {
			name = boardTable[i].name;
			board_id = i;
			break;
		}
	}
	M5.Display.startWrite();
	M5.Display.print("Core:");
	M5.Display.println(name);
	Serial.printf("core:%s\n", name);

	switch (M5.Imu.getType()) {
	case m5::imu_t::imu_mpu6050:	name = "MPU6050";	break;
	case m5::imu_t::imu_mpu6886:	name = "MPU6886";	break;
	case m5::imu_t::imu_mpu9250:	name = "MPU9250";	break;
	case m5::imu_t::imu_sh200q:		name = "SH200Q";	break;
	default:						name = "none";		break;
	}
	M5.Display.print("IMU:");
	M5.Display.println(name);
	M5.Display.endWrite();
	Serial.printf("imu:%s\n", name);

	espcamera_setup();
	initWifi(ver, false, onConnect);
}

void _loop(void)
{
	espcamera_loop();
	vTaskDelay(1);
	M5.update();  // update button and speaker
}
