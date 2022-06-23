// copyright to SohtaMei 2019.

#include <stdint.h>
#include <Arduino.h>
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

#define PORT_TONE	8
#define PORT_CH4F	9
#define PORT_CH4R	10

static volatile int calibLR;
#define EEPROM_CALIB	0x00

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

	TCCR0A=0x03; TCCR0B=0x03;	// timer0:8bit高速PWM, 1/64(977Hz), PWM6,5/timer
	TCCR1A=0x01; TCCR1B=0x0B;	// timer1:8bit高速PWM, 1/64(977Hz), PWM9,10/servo
	TCCR2A=0x03; TCCR2B=0x04;	// timer2:8bit高速PWM, 1/64(977Hz), PWM11,3/buzzer

	calibLR = EEPROM.read(EEPROM_CALIB);
	if(calibLR >= 0x80) calibLR -= 0x100;
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
/*
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
*/
uint16_t remoconRobo_getAnalog(uint8_t ch, uint16_t count, uint8_t discharge)
{
	if(discharge) {
		pinMode(ch, OUTPUT);		// 電荷discharge (リモコンロボで1000mVくらいに帯電してしまう)
		digitalWrite(ch, LOW);
		delay(1);
	}
	pinMode(ch, INPUT);

	if(count == 0) count = 1;
	count *= 100;
	uint32_t sum = 0;
	for(int i = 0; i < count; i++)
		sum += analogRead(ch);
	return ((sum / count) * 625UL) / 128;	// 1024->5000
}
