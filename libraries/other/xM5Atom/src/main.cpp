/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include <TukurutchEsp.h>
#include <M5Atom.h>
#include <Adafruit_VL53L0X.h>
#include "main.h"


WebsocketsServer wsServer;

#define numof(a) (sizeof(a)/sizeof((a)[0]))

void _setLED(uint8_t onoff)
{
	if(onoff)
		M5.dis.drawpix(0,0xf00000);
	else
		M5.dis.clear();
}

uint8_t _getSw(uint8_t button)
{
	switch(button) {
	case 0: return M5.Btn.isPressed();
//	case 1: return M5.BtnB.isPressed();
//	case 2: return M5.BtnC.isPressed();
	}
	return 0;
}

float getIMU(uint8_t index)
{
	float data[3]={0};
	if(index < 3) {
		M5.IMU.getGyroData(data+0,data+1,data+2);
		return data[index-0];
	} else if(index < 6) {
		M5.IMU.getAccelData(data+0,data+1,data+2);
		return data[index-3];
	} else if(index < 9) {
		//M5.IMU.getAhrsData(data+0,data+1,data+2);
		return data[index-6];
	} else {
		M5.IMU.getTempData(data+0);
		return data[0];
	}
}

static void onConnect(String ip)
{
	_setLED(1);
	wsServer.listen(PORT_WEBSOCKET);
	Serial.println(ip);
}

void _setup(const char* ver)
{
	M5.begin(true, true, true); // init lcd, power, serial
	M5.IMU.Init();
	Serial.begin(115200);
	initWifi(ver, false, onConnect);
}

void _loop(void)
{
	M5.update();  // update button and speaker
//	delay(50);
}


//------------------------

Adafruit_VL53L0X lox0 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
VL53L0X_RangingMeasurementData_t measure0;
VL53L0X_RangingMeasurementData_t measure1;

static uint8_t tofEnabled[2] = {0,0};

void initTOF(uint8_t tof_num)
{
	Wire.begin(25,21);   // bottom
	if(lox0.begin(VL53L0X_I2C_ADDR, false, &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED)) {
		lox0.startRangeContinuous(100);
		tofEnabled[0] = 1;
	} else {
		Serial.println(F("Failed to boot VL53L0X 0"));
	}

	if(tof_num >= 2) {
		Wire1.begin(26,32);  // grobe
		if(lox1.begin(VL53L0X_I2C_ADDR, false, &Wire1, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED)) {
			lox1.startRangeContinuous(100);
			tofEnabled[1] = 1;
		} else {
			Serial.println(F("Failed to boot VL53L0X 1"));
		}
	}
}

int getTOF(uint8_t index)
{
	if(index >= 2 || tofEnabled[index] == 0)
		return -1;

#if 1
	if(index == 0) {
		while(!lox0.isRangeComplete());
//		if(!lox0.isRangeComplete()) return -1;
		return lox0.readRangeResult();
//		return lox0.readRange();
	} else {
		while(!lox1.isRangeComplete());
//		if(!lox1.isRangeComplete()) return -1;
		return lox1.readRangeResult();
//		return lox1.readRange();
	}
#else
	if(index == 0) {
		lox0.rangingTest(&measure0, false); // pass in 'true' to get debug data printout!
		if (measure0.RangeStatus == 4) return -1;	// out of range
		return measure0.RangeMilliMeter;
	} else {
		lox1.rangingTest(&measure1, false);
		if (measure1.RangeStatus == 4) return -1;
		return measure1.RangeMilliMeter;
	}
#endif
}

int getTOFs(uint8_t* buf)
{
	if(tofEnabled[0] == 0) return 0;
	SetL16(buf+0, getTOF(0));

	if(tofEnabled[1] == 0) return 2;
	SetL16(buf+2, getTOF(1));
	return 4;
}
