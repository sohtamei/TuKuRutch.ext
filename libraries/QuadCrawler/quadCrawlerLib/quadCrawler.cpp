// copyright to SohtaMei & H.Sunakawa 2020.

#include <stdint.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include "quadCrawler.h"

// ポート定義

#define P_Bz		8	// Bzzer Pin
#define P_Echo		A0	// P_Echo Pin
#define P_Trig		A1	// Trigger Pin
#define P_Moter_EN	A3	// Surbo Moter Drive Enable Pin
#define P_Neopix	A2

#define EEPROM_CALIB	0x00
static int8_t calib[8] = {0,0,0,0,0,0,0,0};

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVO_MIN 90L   // Min pulse length out of 4096 (実測 75)
#define SERVO_MAX 545L  // Max pulse length out of 4096 (実測 560)
#define DEG_MIN   -5L   // Min deg
#define DEG_MAX   185L  // Max deg (実測190度)

enum {
	RFK = 0,	// right-front-knee
	RFC,		//            -crotch
	RRK,		//      -rear
	RRC,
	LFK,		// left
	LFC,
	LRK,
	LRC,
};

#define INV		127		// invalid

// knee
#define K_min	(-35)	// min(up)
#define K_max	(100)	// max(down)

#define K_U		(-25)
#define K_D		( -5)
#define K_D2	( 60)

// crotch
#define C_Fmin	(-55)	// frontLeg-front
#define C_Fmax	( 60)	//         -rear
#define C_Rmin	(-90)	// rearLeg
#define C_Rmax	( 45)

// crotch walk position
#define C_FB1	(0)		// FW/BW
#define C_FB2	(55)
#define C_LR1	(0)		// Left/right
#define C_LR2	(45)
#define C_CW	(25)	// cw/ccw

// neutral
#define RFK_N	79
#define RRK_N	77
#define LFK_N	94
#define LRK_N	94

#define RFC_N	90
#define RRC_N	94
#define LFC_N	94
#define LRC_N	94

enum {
	ServoOff = 0,
	ServoSet,
	ServoWalk,

	ServoNeutral,
	ServoRepeat0,
	ServoRepeat1,
};

static uint8_t  servoState = 0;
static uint32_t servoTime = 0;
static uint16_t servoDelay = quadCrawler_fast;
static uint8_t cur_com = COM_STOP;
extern volatile unsigned long timer0_millis;

static int8_t servoStart[8] = {0,0,0,0,0,0,0,0};
static int8_t servoEnd[8] = {-1,-1,-1,-1,-1,-1,-1,-1};

//---------------------------------------------------------

	static const uint8_t channelTbl[8]	= {1,		0,		5,		4,		3,		2,		7,		6};

static void _set_servo(uint8_t id, int16_t deg)
{
										// RFK		RFC		RRK		RRC		LFK		LFC		LRK		LRC
	static const  int8_t polTbl[8]		= {+1,		-1,		+1,		-1,		-1,		+1,		-1,		+1};
	static const  int8_t neutralTbl[8]	= {RFK_N,	RFC_N,	RRK_N,	RRC_N,	LFK_N,	LFC_N,	LRK_N,	LRC_N};
	static const  int8_t minTbl[8]		= {K_min,	C_Fmin,	K_min,	C_Rmin,	K_min,	C_Fmin,	K_min,	C_Rmin};
	static const  int8_t maxTbl[8]		= {K_max,	C_Fmax,	K_max,	C_Rmax,	K_max,	C_Fmax,	K_max,	C_Rmax};

	if(id >= sizeof(channelTbl)) return;

	if(deg > maxTbl[id]) deg = maxTbl[id];
	else if(deg < minTbl[id]) deg = minTbl[id];

	deg = (deg + calib[id]) * polTbl[id] + neutralTbl[id];
	if(deg > DEG_MAX) deg = DEG_MAX;
	else if(deg < DEG_MIN) deg = DEG_MIN;

	pwm.setPWM(channelTbl[id], 0, ((deg-DEG_MIN)*(SERVO_MAX-SERVO_MIN)) / (DEG_MAX-DEG_MIN) + SERVO_MIN);
}

void _setPWM(uint8_t id, uint16_t value)
{
	pwm.setPWM(channelTbl[id], 0, value);
}

static void setServo1(uint8_t id, int16_t deg)
{
	_set_servo(id, deg);
	servoEnd[id] = deg;
}

static void setServo(const int8_t motion[8])
{
	uint8_t i;
	for(i = 0; i < 8; i++)
		if(motion[i] != INV) setServo1(i, motion[i]);
	servoState = ServoSet;
	servoTime = timer0_millis;
}

static void setServoOff(void)
{
	uint8_t i;
	for(i = 0; i < 8; i++)
		pwm.setPWM(i, 0, 0);
	servoState = ServoOff;
	cur_com = COM_STOP;
}

//---------------------------------------------------------

static void setTarget(const int8_t motion[8], uint8_t state)
{
	uint8_t i;
	for(i = 0; i < 8; i++) {
		servoStart[i] = servoEnd[i];
		if(motion[i] != INV)
			servoEnd[i] = motion[i];
	}
	servoState = state;
	servoTime = timer0_millis;
}

static void setTargetLoop(int elapsed)
{
	int32_t k256 = (elapsed * 256)/servoDelay;		// 距離の係数*256
	uint8_t i;
	for(i = 0; i < 8; i++) {
		if(servoEnd[i] != servoStart[i])
			_set_servo(i, ((((int)servoEnd[i] - (int)servoStart[i]) * k256)>>8) + servoStart[i]);
	}
}

static const int8_t (*repeatMotion)[8] = NULL;
static void repeatServo(const int8_t (*motion)[8])
{
	repeatMotion = motion;
	setTarget(motion[0], ServoRepeat0);
}

static void neutralServo()
{
	servoDelay = quadCrawler_fast;
	static const int8_t motion[] = {0,0,0,0,0,0,0,0};
	setTarget(motion, ServoNeutral);
}

//---------------------------------------------------------

typedef struct { int8_t RF; int8_t LF; int8_t RR; int8_t LR; } s_walk;
static const s_walk* walkTable = NULL;

typedef struct { int16_t RF; int16_t LF; int16_t RR; int16_t LR; } s_walkCalib;
static s_walkCalib walkCalib256;
static const s_walkCalib walkCalibInit = {256,256,256,256};

static uint8_t firstFlag = 0;
static void walkServo(s_walk pWalk[2])
{
	walkTable = pWalk;
	servoState = ServoWalk;
	servoTime = timer0_millis;
	firstFlag = 1;
}

static void walkServoLoop(uint16_t elapsed)
{
	#define SwingDuration1 30		// up
	#define SwingDuration2 100		// swing
	#define SwingDuration3 140		// down

	if(firstFlag) {
		if(elapsed < SwingDuration1) {
			setServo1(LFK, K_U);
			setServo1(RRK, K_U);
			return;
		} else if(elapsed < SwingDuration2) {
			setServo1(LFC, walkTable[0].LF + walkTable[1].LF/2);
			setServo1(RRC, walkTable[0].RR + walkTable[1].RR/2);
			return;
		} else if(elapsed < SwingDuration3) {
			setServo1(LFK, K_D);
			setServo1(RRK, K_D);
			return;
		} else {
			firstFlag = 0;
			servoTime = timer0_millis;
			elapsed = 0;
		}
	}

	int32_t t;
	t = elapsed;
	if(t < SwingDuration1) {
		setServo1(RFK, K_U);
		setServo1(LRK, K_U);
	} else if(t < SwingDuration2) {
		setServo1(RFC, walkTable[0].RF);
		setServo1(LRC, walkTable[0].LR);
	} else {
		int32_t k256 = ((t-SwingDuration2)*256)/(servoDelay*4);
		setServo1(RFK, K_D);
		setServo1(LRK, K_D);
		setServo1(RFC, walkTable[0].RF + (walkTable[1].RF*k256*walkCalib256.RF)/65536);
		setServo1(LRC, walkTable[0].LR + (walkTable[1].LR*k256*walkCalib256.LR)/65536);
	}

	t = elapsed + (SwingDuration2 + (servoDelay*4))/2;
	if(t >= SwingDuration2 + (servoDelay*4))
		t -= SwingDuration2 + (servoDelay*4);

	if(t < SwingDuration1) {
		setServo1(LFK, K_U);
		setServo1(RRK, K_U);
	} else if(t < SwingDuration2) {
		setServo1(LFC, walkTable[0].LF);
		setServo1(RRC, walkTable[0].RR);
	} else {
		int32_t k256 = ((t-SwingDuration2)*256)/(servoDelay*4);
		setServo1(LFK, K_D);
		setServo1(RRK, K_D);
		setServo1(LFC, walkTable[0].LF + (walkTable[1].LF*k256*walkCalib256.LF)/65536);
		setServo1(RRC, walkTable[0].RR + (walkTable[1].RR*k256*walkCalib256.RR)/65536);
	}

	if(elapsed >= SwingDuration2 + (servoDelay*4))
		servoTime = timer0_millis;
}

//---------------------------------------------------------

void quadCrawler_setPose1(uint8_t id, int8_t knee, int8_t crotch)
{
	if(id >= 4) return;

	setServo1(id*2+0, knee);
	setServo1(id*2+1, crotch);
	servoState = ServoSet;
	servoTime = timer0_millis;
	cur_com = COM_POSE;
}

void quadCrawler_setPose4(
	int8_t rfk, int8_t rfc,
	int8_t rrk, int8_t rrc,
	int8_t lfk, int8_t lfc,
	int8_t lrk, int8_t lrc)
{
	quadCrawler_setPose1(0, rfk, rfc);
	quadCrawler_setPose1(1, rrk, rrc);
	quadCrawler_setPose1(2, lfk, lfc);
	quadCrawler_setPose1(3, lrk, lrc);
}

uint8_t quadCrawler_checkServoON(void)
{
	return (servoState != ServoOff);
}

void quadCrawler_servoLoop(void)
{
	int32_t elapsed = (timer0_millis - servoTime) & 0x7FFFFFFF;
	switch(servoState) {
	case ServoOff:
		break;

	// 固定姿勢: 赤点滅、180秒後にサーボOFF
	case ServoSet:
		if(elapsed >= 180*1000UL) {
			setServoOff();
			digitalWrite(P_LED, 0);
			return;
		}
		if((elapsed >> 8) & 1)      // 256
			digitalWrite(P_LED, 1);
		else
			digitalWrite(P_LED, 0);
		break;

	case ServoWalk:
		walkServoLoop(elapsed);
		break;

	case ServoRepeat0:
		if(elapsed < servoDelay)
			setTargetLoop(elapsed);
		else
			setTarget(repeatMotion[1], ServoRepeat1);
		break;

	case ServoRepeat1:
		if(elapsed < servoDelay)
			setTargetLoop(elapsed);
		else
			setTarget(repeatMotion[0], ServoRepeat0);
		break;

	case ServoNeutral:
		if(elapsed < servoDelay)
			setTargetLoop(elapsed);
		else
			setServoOff();
		break;

	default:
		break;
	}
}

void quadCrawler_setSpeed(uint16_t speed)
{
	servoDelay = speed;
	memcpy(&walkCalib256, &walkCalibInit, sizeof(walkCalib256));
}

void quadCrawler_setSpeed(uint16_t speed, int16_t x, int16_t y)
{
	servoDelay = speed;
	memcpy(&walkCalib256, &walkCalibInit, sizeof(walkCalib256));

	if(x == 0 || y == 0) return;

	// x/yで進行方向 微調整
	int16_t calib;
	switch(cur_com) {
	case COM_RW:
		x = -x;
	case COM_FW:
		calib = x*256L*2/y;
		if(calib>=0)
			walkCalib256.RF = walkCalib256.RR = 256 - calib;
		else
			walkCalib256.LF = walkCalib256.LR = 256 + calib;
		break;

	case COM_RIGHT:
		y = -y;
	case COM_LEFT:
		calib = y*256L*2/x;
		if(calib>=0)
			walkCalib256.LR = walkCalib256.RR = 256 - calib;
		else
			walkCalib256.LF = walkCalib256.RF = 256 + calib;
		break;
	}
}

void quadCrawler_Walk(uint16_t speed, uint8_t com)
{
	quadCrawler_setSpeed(speed);

	if (cur_com == com) return;
	cur_com = com;

	switch(com) {
	case COM_STOP:
		// normal, repeatのとき : neutral姿勢
		if(servoState != ServoNeutral && servoState != ServoOff)
			neutralServo();
		break;
									//	RF				LF				RR				LR
	case COM_FW: {
		static const s_walk walk[2] = {	{+C_FB1,		+C_FB1,			-C_FB2,			-C_FB2},
										{+(C_FB2-C_FB1),+(C_FB2-C_FB1),	-(C_FB1-C_FB2),	-(C_FB1-C_FB2)} };	walkServo(walk);	break; }
	case COM_RW: {
		static const s_walk walk[2] = {	{+C_FB2,		+C_FB2,			-C_FB1,			-C_FB1},
										{+(C_FB1-C_FB2),+(C_FB1-C_FB2),	-(C_FB2-C_FB1),	-(C_FB2-C_FB1)} };	walkServo(walk);	break; }
	case COM_LEFT: {
		static const s_walk walk[2] = {	{-C_LR2,		-C_LR1,			+C_LR2,			+C_LR1},
										{-(C_LR1-C_LR2),-(C_LR2-C_LR1),	+(C_LR1-C_LR2),	+(C_LR2-C_LR1)} };	walkServo(walk);	break; }
	case COM_RIGHT: {
		static const s_walk walk[2] = {	{-C_LR1,		-C_LR2,			+C_LR1,			+C_LR2},
										{-(C_LR2-C_LR1),-(C_LR1-C_LR2),	+(C_LR2-C_LR1),	+(C_LR1-C_LR2)} };	walkServo(walk);	break; }
	case COM_CCW: {
		static const s_walk walk[2] = {	{-C_CW,			+C_CW,			-C_CW,			+C_CW},
										{+C_CW*2,		-C_CW*2,		+C_CW*2,		-C_CW*2} };		walkServo(walk);	break; }
	case COM_CW: {
		static const s_walk walk[2] = {	{+C_CW,			-C_CW,			+C_CW,			-C_CW},
										{-C_CW*2,		+C_CW*2,		-C_CW*2,		+C_CW*2} };		walkServo(walk);	break; }

										//	RF			RR			LF			LR
	case COM_NEUTRAL: {
		static const int8_t motion[] =		{0,0,		0,0,		0,0,		0,0};		setServo(motion);	break; }
	case COM_ALL_UP: {
		static const int8_t motion[] =		{K_U, INV,	K_U, INV,	K_U, INV,	K_U, INV};	setServo(motion);	break; }
	case COM_ALL_DOWN: {
		static const int8_t motion[] =		{K_D2,INV,	K_D2,INV,	K_D2,INV,	K_D2,INV};	setServo(motion);	break; }
	case COM_T_DOWN: {
		static const int8_t motion[] =		{K_D, INV,	K_D2,INV,	K_D, INV,	K_D2,INV};	setServo(motion);	break; }
	case COM_H_DOWN: {
		static const int8_t motion[] =		{K_D2,INV,	K_D, INV,	K_D2,INV,	K_D, INV};	setServo(motion);	break; }
	case COM_L_DOWN: {
		static const int8_t motion[] =		{K_D, INV,	K_D, INV,	K_D2,INV,	K_D2,INV};	setServo(motion);	break; }
	case COM_R_DOWN: {
		static const int8_t motion[] =		{K_D2,INV,	K_D2,INV,	K_D, INV,	K_D, INV};	setServo(motion);	break; }

	case COM_T_UPDOWN: {
		static const int8_t motion[2][8] = {{K_D, INV,	K_D, INV,	K_D, INV,	K_D, INV},
											{K_D, INV,	K_D2,INV,	K_D, INV,	K_D2,INV}};	repeatServo(motion);	break; }
	case COM_L_R_UP: {
		static const int8_t motion[2][8] = {{K_D2,INV,	K_D2,INV,	K_D, INV,	K_D, INV},
											{K_D, INV,	K_D, INV,	K_D2,INV,	K_D2,INV}};	repeatServo(motion);	break; }
	case COM_ALL_UPDOWN: {
		static const int8_t motion[2][8] = {{K_D, INV,	K_D, INV,	K_D, INV,	K_D, INV},
											{K_D2,INV,	K_D2,INV,	K_D2,INV,	K_D2,INV}};	repeatServo(motion);	break; }
	}
}

int16_t _calibServo(uint8_t id, uint8_t cmd)
{
	if(id>8) return 0;

	switch(cmd) {
	case CALIB_GET:
		return calib[id];

	case CALIB_INC:
		if(calib[id] < 127)
			calib[id]++;
		break;

	case CALIB_DEC:
		if(calib[id] > -128)
			calib[id]--;
		break;

	case CALIB_RESET:
		calib[id] = 0;
		break;

	case CALIB_RESET_ALL:
		memset(calib, 0, sizeof(calib));
		for(int i = 0; i < 8; i++)
			EEPROM.update(EEPROM_CALIB+i, calib[i]);
		return 0;
	}
	EEPROM.update(EEPROM_CALIB+id, calib[id]);
	setServo1(id, 0);

	servoState = ServoSet;
	servoTime = timer0_millis;
	cur_com = COM_POSE;
	return calib[id];
}

//---------------------------------------------------------

Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, P_Neopix, NEO_GRB + NEO_KHZ800);

static void colorWipe(uint32_t c)
{
	uint8_t i;
	for (i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, c);
		strip.show();
		delay(10);
	}
}

void quadCrawler_colorWipe(uint8_t color)
{
	switch(color) {
	case COLOR_OFF:
		colorWipe(strip.Color(0,0,0));
		break;
	case COLOR_RED:
		colorWipe(strip.Color(255,0,0));
		break;
	case COLOR_GREEN:
		colorWipe(strip.Color(0,255,0));
		break;
	case COLOR_BLUE:
		colorWipe(strip.Color(0,0,255));
		break;
	case COLOR_YELLOW:
		colorWipe(strip.Color(128,128,0));
		break;
	case COLOR_PURPLE:
		colorWipe(strip.Color(128,0,128));
		break;
	case COLOR_LIGHTBLUE:
		colorWipe(strip.Color(0,128,128));
		break;
	default:
		break;
	}
}

void quadCrawler_colorRed(uint8_t id)
{
	colorWipe(strip.Color(0,0,0));

	strip.setPixelColor(id*2+0, strip.Color(255,0,0));
	strip.show();
	delay(10);

	strip.setPixelColor(id*2+1, strip.Color(255,0,0));
	strip.show();
	delay(10);
}

static uint32_t Wheel(byte WheelPos)
{
	WheelPos = 255 - WheelPos;
	if (WheelPos < 85) {
		return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	if (WheelPos < 170) {
		WheelPos -= 85;
		return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
	WheelPos -= 170;
	return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void quadCrawler_rainbow(uint8_t wait)
{
	uint16_t i, j;

	for (j = 0; j < 256; j++) {
		for (i = 0; i < strip.numPixels(); i++) {
			strip.setPixelColor(i, Wheel((i + j) & 255));
		}
		strip.show();
		delay(wait);
	}
}

//---------------------------------------------------------

double quadCrawler_getSonner()
{
	if(digitalRead(P_Echo) == HIGH)
		return 100.0;

	double data;
	digitalWrite(P_Trig, LOW);	delayMicroseconds(2);
	digitalWrite(P_Trig, HIGH);	delayMicroseconds(10);
	digitalWrite(P_Trig, LOW);

	data = pulseIn(P_Echo, HIGH);
	if (data > 0) {
		return data * 0.017;
	} else {
		return 0;
	}
}

void quadCrawler_beep(int time)
{
	for (int i = 0; i < time; i++) {
		digitalWrite(P_Bz, HIGH);
		delayMicroseconds(400);
		digitalWrite(P_Bz, LOW);
		delayMicroseconds(400);
	}
}


void quadCrawler_init(void)
{
	pinMode(P_Moter_EN, OUTPUT);
	digitalWrite(P_Moter_EN, HIGH);

	pinMode(P_Echo, INPUT_PULLUP);
	pinMode(P_Trig, OUTPUT);
	pinMode(P_Bz, OUTPUT);
	pinMode(P_Sw1, INPUT_PULLUP);
	pinMode(P_Sw2, INPUT_PULLUP);
	pinMode(P_Sw3, INPUT_PULLUP);
	pinMode(P_Sw4, INPUT_PULLUP);

	strip.begin();
	strip.show(); // Initialize all pixels to 'off'

	uint8_t i;
	uint8_t initFlag = 1;
	for(i = 0; i < 8; i++) {
		calib[i] = EEPROM.read(EEPROM_CALIB+i);
		if(calib[i] != -1) initFlag = 0;
	}
	if(initFlag) {
		Serial.println("init calib");
		memset(calib, 0, sizeof(calib));
		for(i = 0; i < 8; i++)
			EEPROM.update(EEPROM_CALIB+i, calib[i]);
	}

	pwm.begin();
	pwm.setPWMFreq(50);  // Analog servos run at ~50 Hz updates
	digitalWrite(P_Moter_EN, LOW);
	neutralServo();
	setTargetLoop(0);
}

