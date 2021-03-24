/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>
#include "main.h"
//#include <Adafruit_Microbit.h>
#include "microbit_Screen.h"

#include <LSM303AGR_ACC_Sensor.h>
//nclude <LSM303AGR_MAG_Sensor.h>

microbit_Screen SCREEN;

//Adafruit_Microbit_Matrix microbit;
LSM303AGR_ACC_Sensor Acc(&Wire);
//LSM303AGR_MAG_Sensor Mag(&Wire);

#define PI  (3.141592653589793)

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

static uint32_t curStatus = 0;
static uint32_t lastStatus = 0;
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

static void irq_buttonA(void) { eventFlag |= EVENT_BUTTONA; }
static void irq_buttonB(void) { eventFlag |= EVENT_BUTTONB; }
static const uint8_t InitLeds[5] = {B00000,B01010,B00000,B10001,B01110};

void _setup(const char* ver)
{
	Serial.begin(115200);
//	microbit.begin();
	SCREEN.begin();
	Wire.begin();
	Acc.begin();
	Acc.Enable();
//	Acc.EnableTemperatureSensor();
//	Mag.begin();
//	Mag.Enable();

	pinMode(0, INPUT_PULLUP);
	pinMode(1, INPUT_PULLUP);
	pinMode(2, INPUT_PULLUP);
	pinMode(PIN_BUTTON_A, INPUT);
	pinMode(PIN_BUTTON_B, INPUT);

	attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_A), irq_buttonA, FALLING);
	attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_B), irq_buttonB, FALLING);
	SCREEN.show(InitLeds);
}

static int16_t tiltX = 0;
static int16_t tiltY = 0;
static int32_t lastAccel[3] = {0};

uint8_t led_count = 0;
void _loop(void)
{
	int i;
	curStatus = 0;

	// port0,1,2 : H->L
	curStatus |= ((digitalRead(2)<<2)|(digitalRead(1)<<1)|(digitalRead(0)<<0)) ^ B111;

	// accel
	int32_t accel[3];		// mg
	Acc.GetAxes(accel);
	tiltX =  atan2(accel[0], -accel[2]) * (180.0 * 10.0 / PI);
	tiltY = -atan2(accel[1], -accel[2]) * (180.0 * 10.0 / PI);

	uint32_t sumAccel = 0;
	for(i = 0; i < 3; i++)
		sumAccel += abs(accel[i] - lastAccel[i]);
	if(sumAccel >=  400) curStatus |= EVENT_MOVED;
	if(sumAccel >= 2000) curStatus |= EVENT_SHAKEN;
	memcpy(lastAccel, accel, sizeof(lastAccel));

	eventFlag |= (lastStatus ^ curStatus) & curStatus;
	lastStatus = curStatus;
#if 0
char buf[100] = {0};
int ret = snprintf(buf,64,"%5ld %5ld %5ld %4ld %5ld %5ld ", accel[0], accel[1], accel[2], tiltX/10, tiltY/10, sumAccel);
for(i=0;i<12;i++) buf[ret+i] = (curStatus & (1<<(11-i))) ? '1': '0';
Serial.println(buf);
#endif
//	float temp;				// C
//	Acc.GetTemperature(&temp);
//	int32_t mag[3];			// mGauss
//	Mag.GetAxes(mag);

	for(i = 0; i < 10; i++) {
		SCREEN.loopScreen();
		delay(1);
	}
}

void _displayText(char* text)
{
	SCREEN.showString(text);  

//	microbit.print(text);
//	microbit.clear();
}

void _displayLed(uint32_t bitmap)
{
	uint8_t ledBuf[5];
	int i;
	for(i = 0; i < 5; i++)
		ledBuf[i] = (bitmap>>i*5) & 0x1F;
//	microbit.show(ledBuf);
	SCREEN.show(ledBuf);
}

void _getData(void)
{
	uint8_t buf[8]={0};

	uint32_t evt = eventFlag; eventFlag = 0;
	buf[0] = evt;
	buf[1] = evt>>8;
	buf[2] = (digitalRead(PIN_BUTTON_A)^1) | ((digitalRead(PIN_BUTTON_B)^1)<<1);
	buf[3] = 0;
	buf[4] = tiltX;
	buf[5] = tiltX>>8;
	buf[6] = tiltY;
	buf[7] = tiltY>>8;

	sendBin(buf,8);
}
