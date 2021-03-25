#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>

#include "main.h"

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
DHT dht(P_TEMP_HUM, DHT11);		// DHT 11
BMP280 bmp280;

#include <LIS3DHTR.h>

LIS3DHTR<TwoWire> LIS;			//Hardware I2C

float _getAccel(uint8_t xyz)
{
	float accel = 0;
	switch(xyz) {
	case 0: accel = LIS.getAccelerationX(); break;
	case 1: accel = LIS.getAccelerationY(); break;
	case 2: accel = LIS.getAccelerationZ(); break;
	}
	return accel;
}

enum {
	T_C4=262, T_D4=294, T_E4=330, T_F4=349, T_G4=392, T_A4=440, T_B4=494,
	T_C5=523, T_D5=587, T_E5=659, T_F5=698,
};

void _tone(int sound, int ms) {
	tone(P_BUZZER, sound, ms);
	delay(ms);
}

void _setup(const char* ver)
{
	pinMode(P_BUZZER, OUTPUT);

	_tone(T_C5, 100);
	Serial.begin(115200);

	u8x8.setBusClock(100000);  // If you breakout other modules, please enable this line
	u8x8.begin();
	u8x8.setFlipMode(1);
	u8x8.setFont(u8x8_font_chroma48medium8_r);
	u8x8.println("Hello !");

	dht.begin();
	bmp280.init();

	LIS.begin(Wire, 0x19); //IIC init
	delay(100);
	LIS.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);
}

void _loop(void)
{
}
