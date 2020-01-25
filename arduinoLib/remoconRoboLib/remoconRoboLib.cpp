// copyright to SohtaMei 2019.

#include <stdint.h>
#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <EEPROM.h>

#include "remoconRoboLib.h"

struct MotorPort {
	uint8_t pwm;
	uint8_t dir;
} static const MotorPort[3] = {
//   pwm dir
	{5,  4},		// CH1 R
	{6,  7},		// CH2 L
	{11, 12},		// CH3
};

#define PORT_IR_RX	2		// INT0
#define PORT_TONE	8
#define PORT_CH4F	9
#define PORT_CH4R	10
#define PORT_LED	13

static volatile int calibLR;
#define EEPROM_CALIB	0x00

/*
uint32_t debug[100];
uint8_t debug2[100];
int debugCnt = 0;
*/

enum {
	STATE_H_IDLE,

	// digital
	STATE_L_HDR,
	STATE_H_HDR,
	STATE_L_BIT,
	STATE_H_BIT,
	STATE_L_STOP,

	// analog
	STATE_ANALOG_DATA,
};
static volatile uint8_t state;
static volatile uint8_t rawCount;
static volatile uint32_t rawData;

static volatile uint32_t last_timer;
static volatile uint32_t last_timer2 = 0;
static volatile uint32_t last_timer3 = 0;

static struct remoconData remoconData1;
static struct remoconData remoconData2;
static int updated = 0;


// nec remote --------------------------------------------------

#define MATCH(ticks, desired_us) \
	 ( ticks >= (desired_us) - ((desired_us)>>2)-1 \
	&& ticks <= (desired_us) + ((desired_us)>>2)+1)

#define DUR_T2		562
#define DUR_L_HDR	(DUR_T2*16)
#define DUR_H_HDR	(DUR_T2*8)
#define DUR_H_RPT	(DUR_T2*4)
#define DUR_L_BIT	(DUR_T2*1)
#define DUR_H_BIT1	(DUR_T2*3)
#define DUR_H_BIT0	(DUR_T2*1)
#define DUR_H_TIMEOUT	110UL	// ms

// analog remote ------------------------------------------------

#define MATCH2(ticks, desired_us) \
	 ( ticks >= (desired_us) - (DUR_T/2) \
	&& ticks <= (desired_us) + (DUR_T/2))

#define DUR_T			350
#define DUR_H_TIMEOUT_A	300UL	// ms

#define Y_CENTER		16
#define X_CENTER		16
#define BIT_SIZE		15

union analogRemote {
	uint16_t data;
	struct {
		unsigned int keys		: 3;	// bit2~0  :
		unsigned int x			: 5;	// bit7~3  :
		unsigned int y			: 5;	// bit12~8 :
		unsigned int ch			: 2;	// bit14~13: (1)chA, (2)chB, (0)chC
	} b;
};

static volatile uint8_t analog_ch = 0;

// REMOTE -------

static void irq_int0(void)
{
	uint8_t irdata = digitalRead(PORT_IR_RX);
	uint32_t cur_timer = micros();
	uint16_t diff;// = ((cur_timer - last_timer) & ~0x80000000);
	if(cur_timer - last_timer >= 0x10000UL) {
		diff = 0xFFFF;
	} else {
		diff = cur_timer - last_timer;
	}
	last_timer = cur_timer;

//	debug[debugCnt] = diff;
//	debug2[debugCnt++] = (state<<4)|irdata;
	switch(state) {
	case STATE_H_IDLE:
	case STATE_H_HDR:
	case STATE_H_BIT:
		if(irdata == 1) {
			state = STATE_H_IDLE;
			return;
		}
		break;

	case STATE_L_HDR:
	case STATE_L_BIT:
	case STATE_L_STOP:
		if(irdata == 0) {
			state = STATE_L_STOP;
			return;
		}
		break;

	case STATE_ANALOG_DATA: {
		if(MATCH2(diff, DUR_T)) {
			if(!(rawCount&1))
				rawData = (rawData<<1) | 1;
			rawCount += 1;
		} else if(MATCH2(diff, DUR_T*2)) {
			rawData = (rawData<<1) | 0;
			rawCount += 2;
		} else {
			if(irdata == 0)
				state = STATE_L_HDR;
			else
				state = STATE_H_IDLE;
			return;
		}
		if(rawCount >= BIT_SIZE*2) {
			state = STATE_H_IDLE;
			union analogRemote rData;
			rData.data = rawData;
			if(!analog_ch) {
				analog_ch = rData.b.ch;
			} else if(rData.b.ch == analog_ch) {
				memset(&remoconData1, 0, sizeof(remoconData1));
				remoconData1.keys	= rData.b.keys + BUTTON_A_XY;
				remoconData1.x		= (rData.b.x - X_CENTER)*16;
				remoconData1.y		= (rData.b.y - Y_CENTER)*16;
				last_timer2 = millis();
				updated = REMOTE_ANALOG;
				digitalWrite(PORT_LED, 1);
			}
		}
		return;
	  }
	}

	switch(state) {
	case STATE_H_IDLE:	// H_IDLE -> L_HDR
		state = STATE_L_HDR;
		break;
	case STATE_L_HDR:	// L_HDR -> H_HDR
		if(MATCH(diff, DUR_L_HDR)) {
			state = STATE_H_HDR;
		} else if(MATCH2(diff, DUR_T*3)) {
			state = STATE_ANALOG_DATA;
			rawData = 0;
			rawCount = 0;
		} else {
			state = STATE_H_IDLE;
		}
		break;
	case STATE_H_HDR:	// L_HDR -> H_BIT,L_RPT(H_STOP)
		if(MATCH(diff, DUR_H_HDR)) {
			rawData = 0;
			rawCount = 0;
			state = STATE_L_BIT;
		} else if(MATCH(diff, DUR_H_RPT)) {
			last_timer2 = millis();
			state = STATE_L_STOP;
		} else {
			state = STATE_L_STOP;
		}
		break;
	case STATE_L_BIT:	// H_BIT -> L_BIT,L_IDLE
		if(MATCH(diff, DUR_L_BIT)) {
			if(rawCount >= 32) {
				if((((rawData>>8)^rawData) & 0x00FF00FF) == 0x00FF00FF) {
					memset(&remoconData1, 0, sizeof(remoconData1));
					remoconData1.keys = (rawData>>16) & 0xFF; //0x00FF00FF;;
					last_timer2 = millis();
					updated = REMOTE_YES;
					digitalWrite(PORT_LED, 1);
				}
				state = STATE_H_IDLE;
			} else {
				state = STATE_H_BIT;
			}
		} else {
			state = STATE_H_IDLE;
		}
		break;
	case STATE_H_BIT:	// L_BIT -> H_BIT
		state = STATE_L_BIT;
		rawData = (rawData>>1);
		rawCount++;
		if(MATCH(diff, DUR_H_BIT1)) {
			rawData |= 0x80000000UL;
		} else if(MATCH(diff, DUR_H_BIT0)) {
			;
		} else {
			state = STATE_L_STOP;
		}
		break;
	case STATE_L_STOP:	// H_STOP -> L_IDLE
		state = STATE_H_IDLE;
		break;
	}
}

static void _initRemote(void)
{
	pinMode(PORT_IR_RX,INPUT);
	attachInterrupt(0, irq_int0, CHANGE);

	state = STATE_H_IDLE;
	memset(&remoconData1, 0, sizeof(remoconData1));
}

int remoconRobo_checkRemoteUpdated(void)
{
	if(last_timer2) {
		uint32_t timeout = (remoconData1.keys >= BUTTON_A_XY) ? DUR_H_TIMEOUT_A : DUR_H_TIMEOUT;
		if(millis() - last_timer2 < timeout) {
			if(!remoconData2.keys && remoconData1.keys) {
				updated = REMOTE_YES;
				digitalWrite(PORT_LED, 1);
			}
			remoconData2 = remoconData1;
		} else if(remoconData2.keys) {
			memset(&remoconData2, 0, sizeof(remoconData2));
			updated = REMOTE_YES;
			digitalWrite(PORT_LED, 0);
			last_timer2 = 0;
			last_timer3 = millis();
		}
	} else {
		if(!last_timer3) last_timer3 = millis();
		uint32_t diff = millis() - last_timer3;
		if(remoconData1.keys && diff > DUR_H_TIMEOUT)
			memset(&remoconData1, 0, sizeof(remoconData1));

		diff >>= 7;
	//	char buf[64]; sprintf(buf, "%d\r\n", diff); Serial.print(buf);
		if(diff == 0) {
			;
		} else if((diff % 16) == 0) {	// 2048
			digitalWrite(PORT_LED, 1);
		} else if((diff % 16) == 1) {
			digitalWrite(PORT_LED, 0);
		}
	}
	int _updated = updated;
	updated = 0;
	return _updated;
}

int remoconRobo_checkRemoteKey(void)
{
	remoconRobo_checkRemoteUpdated();
	return remoconData2.keys;
}

struct remoconData remoconRobo_getRemoteData(void)
{
	return remoconData2;
}

int remoconRobo_getRemoteKeys(void)
{
	return remoconData2.keys;
}

int remoconRobo_getRemoteX(void)
{
	return remoconData2.x;
}

int remoconRobo_getRemoteY(void)
{
	return remoconData2.y;
}

int remoconRobo_isRemoteKey(int key)
{
	if(key == 0xFF)
		return remoconData2.keys != 0;
	else
		return remoconData2.keys == key;
}

int remoconRobo_getRemoteCh(void)
{
	return analog_ch;
}

// other -------

void remoconRobo_tone(int sound, int ms)
{
	int TCCR2Alast = TCCR2A;
	int TCCR2Blast = TCCR2B;
	int OCR2Alast = OCR2A;
	tone(PORT_TONE, sound, ms); delay(ms);
	TCCR2A = TCCR2Alast;
	TCCR2B = TCCR2Blast;
	OCR2A = OCR2Alast;
}

void remoconRobo_init(void)
{
	int ch;
	for(ch = 0; ch < 3; ch++)
		pinMode(MotorPort[ch].dir, OUTPUT);

	pinMode(PORT_TONE, OUTPUT);
	pinMode(PORT_LED, OUTPUT);

	TCCR0A=0x03; TCCR0B=0x03;	// timer0:8bit高速PWM, 1/64(977Hz), PWM6,5/timer
	TCCR1A=0x01; TCCR1B=0x0B;	// timer1:8bit高速PWM, 1/64(977Hz), PWM9,10/servo
	TCCR2A=0x03; TCCR2B=0x04;	// timer2:8bit高速PWM, 1/64(977Hz), PWM11,3/buzzer

	_initRemote();
	calibLR = EEPROM.read(EEPROM_CALIB);
	if(calibLR >= 0x80) calibLR -= 0x100;

//	Serial.begin(115200);
}

void remoconRobo_initCh4(void)
{
	pinMode(PORT_CH4F, OUTPUT);
	pinMode(PORT_CH4R, OUTPUT);
}

// Motor -----------

static uint8_t initCh4 = 0;
void remoconRobo_setMotor(int ch, int speed)
{
	if(ch == CH4) {
		if(!initCh4) {
			initCh4 = 1;
			remoconRobo_initCh4();
		}
		digitalWrite(PORT_CH4F, speed > 0 ? 1: 0);
		digitalWrite(PORT_CH4R, speed < 0 ? 1: 0);
	} else {
		if(speed > 255)  speed = 255;
		if(speed < -255) speed = -255;
		if(speed >= 0) {
			digitalWrite(MotorPort[ch].dir, HIGH);
			analogWrite(MotorPort[ch].pwm, speed);
		} else {
			digitalWrite(MotorPort[ch].dir, LOW);
			analogWrite(MotorPort[ch].pwm, -speed);
		}
	}
}

void remoconRobo_setRobotLR(int speedL, int speedR)
{
	if(calibLR >= 0) {
		speedR = (speedR * (256L - calibLR)) >> 8;
	} else {
		speedL = (speedL * (256L + calibLR)) >> 8;
	}
	remoconRobo_setMotor(CH_R,  speedR);
	remoconRobo_setMotor(CH_L, -speedL);
}

struct {
	int  L;
	int  R;
} static const dir_table[6] = {
	//L   R
	{ 1,  1},	// DIR_FORWARD
	{ 0,  1},	// DIR_LEFT,
	{ 1,  0},	// DIR_RIGHT,
	{-1, -1},	// DIR_BACK,
	{-1,  1},	// DIR_ROLL_LEFT,
	{ 1, -1},	// DIR_ROLL_RIGHT,
};

void remoconRobo_setRobot(int direction, int speed)
{
	remoconRobo_setRobotLR(speed * dir_table[direction].L,
						 speed * dir_table[direction].R);
}

int remoconRobo_getCalib(void)
{
	return calibLR;
}

int remoconRobo_setCalib(int calib)
{
	int overflow = 0;
	calibLR = calib;
	if(calibLR > 127) {
		calibLR = 127;
		overflow = -1;
	}
	if(calibLR < -128) {
		calibLR = -128;
		overflow = -1;
	}
	EEPROM.update(EEPROM_CALIB, calibLR & 0xFF);
	return overflow;
}

int remoconRobo_incCalib(int offset)
{
	calibLR += offset;
	return remoconRobo_setCalib(calibLR);
}

// MP3 ----------------

#include "SoftwareSerialRR.h"

static SoftwareSerialRR mySerial(PORT_TONE, 9600); // RX, TX

static uint8_t loop_flag = 1;

#define SendMP3(buf) sendMP3(buf, sizeof(buf))
static void sendMP3(const uint8_t* buf, int size)
{
	int i;

	#define CMD_INTERVAL	200
	mySerial.write(0x7E);
	mySerial.write(size+1);
	for(i=0;i<size;i++) mySerial.write(buf[i]);
	mySerial.write(0xEF);
	delay(CMD_INTERVAL);
}

void remoconRobo_initMP3(int volume)
{
	uint8_t setVol[2] = {0x06,0x00};
	setVol[1] = volume;
	SendMP3(setVol);
	remoconRobo_stopMP3();
}

void remoconRobo_playMP3(int track, int loop)
{
	enum {
		LOOP_ALL,
		LOOP_FOLDER,
		LOOP_ONE,
		LOOP_RAM,
		LOOP_ONE_STOP,
	};

	loop_flag = loop;
	uint8_t loopOne[2] = {0x11,0x02};	// 0-all, 1-folder, 2-oneloop, 3-RAM?, 4-loopOnce
	if(loop)
		loopOne[1] = LOOP_ONE;
	else
		loopOne[1] = LOOP_ONE_STOP;
	SendMP3(loopOne);

//	const uint8_t play[1] = {0x0D};
	uint8_t playN[3] = {0x03,0x00,0x01};
	playN[2] = track;
	SendMP3(playN);
}

void remoconRobo_stopMP3(void)
{
	if(1) {//loop_flag) {
		loop_flag = 0;
		const uint8_t pause[1] = {0x0A};
		SendMP3(pause);
	}
}

uint16_t remoconRobo_getAnalog(uint8_t ch, uint16_t count)
{
	if(count == 0) count = 1;
	uint32_t sum = 0;
	uint16_t i;
	for(i = 0; i < count; i++)
		sum += analogRead(ch);
	sum = ((sum / count) * 625UL) / 128;	// 1024->5000
	return sum;
}
