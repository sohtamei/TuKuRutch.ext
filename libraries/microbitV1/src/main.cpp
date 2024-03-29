/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>
#include "main.h"

#include "microbit_Screen.h"
microbit_Screen SCREEN;

#include "NRF51_Radio_library.h"
NRF51_Radio MicrobitRadio = NRF51_Radio();

// microbit V1.5
#if 1
#include <LSM303AGR_ACC_Sensor.h>
LSM303AGR_ACC_Sensor AccNew(&Wire);
#else
#include <Adafruit_LSM303_Accel.h>
#include <Adafruit_Sensor.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Accel_Unified AccNew = Adafruit_LSM303_Accel_Unified(54321);
#endif

//nclude <LSM303AGR_MAG_Sensor.h>
//LSM303AGR_MAG_Sensor Mag(&Wire);

// microbit V1.3
#include "MMA8653.h"
MMA8653 AccOld;


/*
#define PIN_A0               (0)
#define PIN_A1               (1)
#define PIN_A2               (2)
#define PIN_A3               (3)
#define PIN_A4               (4)
#define PIN_BUTTON_A         (5)
#define PIN_A5               (10)
#define PIN_BUTTON_B         (11)
#define PIN_SPI_SCK          (13)
#define PIN_SPI_MISO         (14)
#define PIN_SPI_MOSI         (15)
#define PIN_WIRE_SCL         (19)
#define PIN_WIRE_SDA         (20)
*/

static uint32_t eventFlag = 0;
enum {
	EVENT_PORT0		= (1<<0),
	EVENT_PORT1		= (1<<1),
	EVENT_PORT2		= (1<<2),
	EVENT_BUTTONA	= (1<<3),
	EVENT_BUTTONB	= (1<<4),

	EVENT_MOVED		= (1<<5),
	EVENT_SHAKEN	= (1<<6),
};

enum {
	BOARDTYPE_NONE = 0,
	BOARDTYPE_OLD,
	BOARDTYPE_NEW,
};
static uint8_t boardType = 0;
#define I2C_ACCEL_OLD	0x1D
#define I2C_ACCEL_NEW	0x19
#define I2C_MAG_OLD		0x0e
#define I2C_MAG_NEW		0x1e

static uint8_t radioEnabled = false;

static void irq_buttonA(void) { eventFlag |= EVENT_BUTTONA; }
static void irq_buttonB(void) { eventFlag |= EVENT_BUTTONB; }
static const uint8_t InitLeds[5] = {B00000,B01010,B00000,B10001,B01110};

void _setup(const char* ver)
{
	// BLE beginすると200cycle程度でエラー。UART RX 1byte受信割り込みがBLE割り込みでoverflowし、
	// エラー状態のままになってしまうものと推察。
	Serial.begin(19200);// 115200);
	Wire.begin();

	pinMode(0, INPUT_PULLUP);
	pinMode(1, INPUT_PULLUP);
	pinMode(2, INPUT_PULLUP);
	pinMode(PIN_BUTTON_A, INPUT);
	pinMode(PIN_BUTTON_B, INPUT);

	attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_A), irq_buttonA, FALLING);
	attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_B), irq_buttonB, FALLING);
	SCREEN.show(InitLeds);

	delay(300);		// for i2c error

	int ret;
	Wire.beginTransmission(I2C_ACCEL_NEW);
	ret = Wire.endTransmission();
	if(!ret) {
		boardType = BOARDTYPE_NEW;
	#if 1
		AccNew.begin();
		AccNew.Enable();
	//	AccNew.EnableTemperatureSensor();
	//	Mag.begin();
	//	Mag.Enable();
	#else
		if(AccNew.begin()) {
			AccNew.setRange(LSM303_RANGE_2G);
			AccNew.setMode(LSM303_MODE_NORMAL);
		} else {
			Serial.println("error");
		}
	#endif
	} else {
		Wire.beginTransmission(I2C_ACCEL_OLD);
		ret = Wire.endTransmission();
		if(!ret) {
			boardType = BOARDTYPE_OLD;
			AccOld.begin(false, 2);		// highres, scale=2G
		} else {
			boardType = BOARDTYPE_NEW;//NONE;
		}
	}
//	Serial.println(boardType);
}

static void _getAccel(int32_t accel[3])
{
	switch(boardType) {
	case BOARDTYPE_NEW:
	#if 1
		AccNew.GetAxes(accel);
	//	float temp;				// C
	//	AccNew.GetTemperature(&temp);
	//	int32_t mag[3];			// mGauss
	//	Mag.GetAxes(mag);
	#else
		sensors_event_t event;
		AccNew.getEvent(&event);
		accel[0] = event.acceleration.x;
		accel[1] = event.acceleration.y;
		accel[2] = event.acceleration.z;
	#endif
		break;
	case BOARDTYPE_OLD:
		AccOld.update();
		accel[0] = AccOld.getX();
		accel[1] = AccOld.getY();
		accel[2] = AccOld.getZ();
		break;
	}
}

static uint32_t lastPort = 0;
void _loop(void)
{
	// port0,1,2 : H->L
	uint32_t curPort = ((digitalRead(2)<<2)|(digitalRead(1)<<1)|(digitalRead(0)<<0)) ^ B111;
	eventFlag |= (lastPort ^ curPort) & curPort;
	lastPort = curPort;

	int i;
	for(i = 0; i < 10; i++) {
		SCREEN.loopScreen();
		delay(1);
	}
}

void _displayText(char* text)
{
	SCREEN.showString(text);  
}

void _displayLed(uint32_t bitmap)
{
	if(!bitmap) {
		SCREEN.disable();
	} else {
		uint8_t ledBuf[5];
		int i;
		for(i = 0; i < 5; i++)
			ledBuf[i] = (bitmap>>i*5) & 0x1F;
		SCREEN.show(ledBuf);
	}
}

int _getTilt(uint8_t xy)
{
	int32_t accel[3] = {0};		// mg
	_getAccel(accel);
	if(xy == 1)
		return -atan2(accel[1], -accel[2]) * (180.0 * 10.0 / PI);	// tiltY
	else
		return  atan2(accel[0], -accel[2]) * (180.0 * 10.0 / PI);	// tiltX
}

static int32_t lastAccel[3] = {0};
void _getSendData(void)
{
	// accel
	int32_t accel[3] = {0};		// mg
	_getAccel(accel);
	int16_t tiltX =  atan2(accel[0], -accel[2]) * (180.0 * 10.0 / PI);
	int16_t tiltY = -atan2(accel[1], -accel[2]) * (180.0 * 10.0 / PI);

	uint32_t sumAccel = 0;
	int i;
	for(i = 0; i < 3; i++)
		sumAccel += abs(accel[i] - lastAccel[i]);
	if(sumAccel >=  400) eventFlag |= EVENT_MOVED;
	if(sumAccel >= 2000) eventFlag |= EVENT_SHAKEN;
	memcpy(lastAccel, accel, sizeof(lastAccel));
  #if 0
	char buf[100] = {0};
	int ret = snprintf(buf,64,"%5ld %5ld %5ld %4ld %5ld %5ld ", accel[0], accel[1], accel[2], tiltX/10, tiltY/10, sumAccel);
	Serial.println(buf);
  #endif
	uint8_t buf[8+36]={0};
	buf[0] = eventFlag;
	buf[1] = eventFlag>>8;
	buf[2] = (digitalRead(PIN_BUTTON_A)^1) | ((digitalRead(PIN_BUTTON_B)^1)<<1);
	buf[3] = 0;
	buf[4] = tiltX;
	buf[5] = tiltX>>8;
	buf[6] = tiltY;
	buf[7] = tiltY>>8;
	eventFlag = 0;

	int size = 8;
	if(radioEnabled) {
		FrameBuffer* recvFrame = MicrobitRadio.recv();
		if(recvFrame) {
			if(recvFrame->length > 36-1)
				recvFrame->length = 36-1;
			memcpy(buf+8, (uint8_t*)recvFrame, recvFrame->length+1);
			size += recvFrame->length+1;
			delete recvFrame;
		}
	}
	sendBin(buf,size);
}

void _RadioEnable(int group, int freq, int power)
{
	if(!radioEnabled) bleStop();
	radioEnabled = true;

	MicrobitRadio.setGroup(group);			// 0~255
	MicrobitRadio.enable();
	MicrobitRadio.setFrequencyBand(freq);	// 0~83. (2400+freq)MHz.
	MicrobitRadio.setTransmitPower(power);	// 0~7
}

void _RadioSend(uint8_t* buf, int size)
{
	if(!radioEnabled) return;
	FrameBuffer sendFrame = {0};
	memcpy(&sendFrame, buf, size);
	MicrobitRadio.send(&sendFrame);
}

void _RadioRecv(void)
{
	FrameBuffer* recvFrame = MicrobitRadio.recv();
	if(recvFrame == NULL) {
		callOK();
		return;
	}

	sendBin((uint8_t*)recvFrame, recvFrame->length+1);
	delete recvFrame;
}
