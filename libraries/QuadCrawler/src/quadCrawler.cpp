// copyright to SohtaMei & H.Sunakawa 2020.

#include <stdint.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include "quadCrawler.h"

// ポート定義

#define P_Echo		A0	// P_Echo Pin
#define P_Trig		A1	// Trigger Pin

#define P_Bz		8	// Bzzer Pin
#define P_Motor_EN	A3	// Surbo Moter Drive Enable Pin

#define P_Neopix	A2

#define EEPROM_CALIB	0x00
static int8_t calib[8] = {0,0,0,0,0,0,0,0};

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVO_MIN 90L   // Min pulse length out of 4096 (実測 75)
#define SERVO_MAX 545L  // Max pulse length out of 4096 (実測 560)
#define DEG_MIN   -5L   // Min deg
#define DEG_MAX   185L  // Max deg (実測190度)

enum {
	RFK = 0,	// 1.right-front
	RFC,
	RRK,		// 2.right-rear
	RRC,
	LFK,		// 3.left-front
	LFC,
	LRK,		// 4.left-rear
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
//#define C_FB1	(0)		// FW/BW
#define C_FB2	(55)
//#define C_LR1	(0)		// Left/right
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
	MotionOff = 0,

	MotionPose,
	MotionRepeat0,
	MotionRepeat1,
	MotionNeutral0,
	MotionNeutral1,

	MotionWalk,
};

static uint8_t cur_com = COM_STOP;

static uint8_t  motionState = MotionOff;
static uint32_t motionStart = 0;
static uint16_t motionDur = quadCrawler_fast;
extern volatile unsigned long timer0_millis;

//---------------------------------------------------------

static int8_t servoPos[8] = {0,0,0,0,0,0,0,0};

	static const uint8_t channelTbl[12]	= {1,		0,		5,		4,		3,		2,		7,		6,	8,9,10,11,};

static void _set_servo(uint8_t id, int16_t deg)
{
	servoPos[id] = deg;
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

	pwm.setPWM(channelTbl[id], (512*id)&0xFFF, (512*id+((deg-DEG_MIN)*(SERVO_MAX-SERVO_MIN)) / (DEG_MAX-DEG_MIN) + SERVO_MIN)&0xFFF);
}

void _setPWM(uint8_t id, uint16_t value)
{
	pwm.setPWM(channelTbl[id], (512*id)&0xFFF, (512*id+value)&0xFFF);
}

//---------------------------------------------------------

static int8_t servoStart[8] = {0,0,0,0,0,0,0,0};
static int8_t servoEnd[8] = {-1,-1,-1,-1,-1,-1,-1,-1};

static uint32_t timeStart[8] = {0,0,0,0,0,0,0,0};
static uint16_t timeDur[8] = {0,0,0,0,0,0,0,0};

static void setTarget(uint8_t id, int16_t deg, int16_t duration)
{
	timeStart[id] = millis();
	timeDur[id] = duration;
	servoStart[id] = servoPos[id];
	servoEnd[id] = deg;
}

static void setTargets(const int8_t motion[8], int16_t duration, uint8_t state)
{
	uint8_t i;
	for(i = 0; i < 8; i++) {
		if(motion[i] != INV)
			setTarget(i, motion[i], duration);
	}
	motionState = state;
	motionStart = millis();
//	Serial.printf("%d %d %d %d %d %d %d %d\n", motion[0], motion[1], motion[2], motion[3], motion[4], motion[5], motion[6], motion[7]);
}

static void setServo1(uint8_t id, int16_t deg)
{
	_set_servo(id, deg);
	servoEnd[id] = deg;
	motionState = MotionPose;
	motionStart = millis();
}

static void setTargetLoop(void)
{
	uint32_t cur = timer0_millis;
	int i;
	for(i = 0; i < 8; i++) {
		if(timeDur[i] == 0) continue;

		int16_t elapsed = cur - timeStart[i];
		if(elapsed > timeDur[i]) continue;

		int32_t k256 = (elapsed*256L)/timeDur[i];
		_set_servo(i, ((((int)servoEnd[i] - (int)servoStart[i]) * k256)>>8) + servoStart[i]);
	}
}

//---------------------------------------------------------

static const int8_t (*repeatMotions)[8] = NULL;
static void motionRepeat(const int8_t (*motion)[8], int16_t duration)
{
	repeatMotions = motion;
	setTargets(motion[0], duration, MotionRepeat0);
}

static void motionNeutral(void)
{
	static const int8_t motion[] = {INV,INV,0,0,0,0,INV,INV};
	setTargets(motion, quadCrawler_fast, MotionNeutral0);
}

static void setMotionOff(void)
{
	uint8_t i;
	for(i = 0; i < 8; i++)
		pwm.setPWM(i, 0, 0);
	motionState = MotionOff;
	cur_com = COM_STOP;
}

static void setServoInit(void)
{
	uint8_t i;
	for(i = 0; i < 8; i++) {
		_set_servo(i, 0);
		delay(40);
	}
	delay(100);
	setMotionOff();
}

//---------------------------------------------------------

typedef struct { int8_t RF; int8_t LF; int8_t RR; int8_t LR; } s_walk;
static const s_walk* walkTable = NULL;

typedef struct { int16_t RF; int16_t LF; int16_t RR; int16_t LR; } s_walkCalib;
static s_walkCalib walkCalib256;
static const s_walkCalib walkCalibInit = {256,256,256,256};

static uint8_t firstFlag = 0;
static void motionWalk(const s_walk pWalk[2])
{
	walkTable = pWalk;
	motionState = MotionWalk;
	motionStart = millis();
	firstFlag = 1;
}

enum {
	DUR_UPDOWN = 80L,
	DUR_UPSLIDE = 120L,
};

enum {
	ST_IDLE,
	ST_UP,
	ST_UPSLIDE,
	ST_DOWNSLIDE,
};

static uint8_t getTargetState(uint16_t elapsed)
{
	if(elapsed < 0+DUR_UPDOWN)
		return ST_UP;
	else if(elapsed < 0+DUR_UPDOWN+DUR_UPSLIDE)
		return ST_UPSLIDE;
	else
		return ST_DOWNSLIDE;
}

static uint8_t LFRR_current = ST_IDLE;
static uint8_t RFLR_current = ST_IDLE;

static void motionWalkLoop(uint16_t elapsed)
{
	uint16_t cycle = DUR_UPDOWN+DUR_UPSLIDE + (motionDur*4);

	uint8_t LFRR_next;
	uint8_t RFLR_next;
	if(firstFlag) {
		LFRR_next = getTargetState(elapsed);
		if(LFRR_current != LFRR_next) {
			LFRR_current = LFRR_next;
			switch(LFRR_current) {
			case ST_UP:
				setTarget(LFK, K_U, DUR_UPDOWN);
				setTarget(RRK, K_U, DUR_UPDOWN);
				break;
			case ST_UPSLIDE:
				setTarget(LFC, walkTable[0].LF + walkTable[1].LF/2, DUR_UPSLIDE);
				setTarget(RRC, walkTable[0].RR + walkTable[1].RR/2, DUR_UPSLIDE);
				break;
			case ST_DOWNSLIDE:
				setTarget(LFK, K_D, DUR_UPDOWN);
				setTarget(RRK, K_D, DUR_UPDOWN);
				setTarget(LFC, walkTable[0].LF + (walkTable[1].LF*walkCalib256.LF)/256, DUR_UPDOWN+cycle/2);
				setTarget(RRC, walkTable[0].RR + (walkTable[1].RR*walkCalib256.RR)/256, DUR_UPDOWN+cycle/2);
				break;
			}
		}
		if(elapsed < DUR_UPDOWN*2+DUR_UPSLIDE) {
			return;
		}
		firstFlag = 0;
		motionStart = millis();
		elapsed = 0;
		RFLR_current = ST_IDLE;
	}

	RFLR_next = getTargetState(elapsed);
	if(RFLR_current != RFLR_next) {
		RFLR_current = RFLR_next;
		switch(RFLR_current) {
		case ST_UP:
			setTarget(RFK, K_U, DUR_UPDOWN);
			setTarget(LRK, K_U, DUR_UPDOWN);
			break;
		case ST_UPSLIDE:
			setTarget(RFC, walkTable[0].RF, DUR_UPSLIDE);
			setTarget(LRC, walkTable[0].LR, DUR_UPSLIDE);
			break;
		case ST_DOWNSLIDE:
			setTarget(RFK, K_D, DUR_UPDOWN);
			setTarget(LRK, K_D, DUR_UPDOWN);
			setTarget(RFC, walkTable[0].RF + (walkTable[1].RF*walkCalib256.RF)/256, motionDur*4);
			setTarget(LRC, walkTable[0].LR + (walkTable[1].LR*walkCalib256.LR)/256, motionDur*4);
			break;
		}
	}

	int32_t t = elapsed + cycle/2;
	if(t >= cycle)
		t -= cycle;

	LFRR_next = getTargetState(t);
	if(LFRR_current != LFRR_next) {
		LFRR_current = LFRR_next;
		switch(LFRR_current) {
		case ST_UP:
			setTarget(LFK, K_U, DUR_UPDOWN);
			setTarget(RRK, K_U, DUR_UPDOWN);
			break;
		case ST_UPSLIDE:
			setTarget(LFC, walkTable[0].LF, DUR_UPSLIDE);
			setTarget(RRC, walkTable[0].RR, DUR_UPSLIDE);
			break;
		case ST_DOWNSLIDE:
			setTarget(LFK, K_D, DUR_UPDOWN);
			setTarget(RRK, K_D, DUR_UPDOWN);
			setTarget(LFC, walkTable[0].LF + (walkTable[1].LF*walkCalib256.LF)/256, motionDur*4);
			setTarget(RRC, walkTable[0].RR + (walkTable[1].RR*walkCalib256.RR)/256, motionDur*4);
			break;
		}
	}

	if(elapsed >= cycle)
		motionStart = millis();
}

//---------------------------------------------------------

void quadCrawler_setPose1(uint8_t id, int8_t knee, int8_t crotch)
{
	if(id >= 4) return;

	setServo1(id*2+0, knee);
	setServo1(id*2+1, crotch);
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
	return (motionState != MotionOff);
}

void quadCrawler_servoLoop(void)
{
	if(motionState == MotionOff) return;

	int32_t elapsed = (timer0_millis - motionStart) & 0x7FFFFFFF;
	switch(motionState) {
	case MotionWalk:
		motionWalkLoop(elapsed);
		break;

	// 固定姿勢: 赤点滅、180秒後にサーボOFF
	case MotionPose:
		if((elapsed >> 8) & 1)      // 256
			digitalWrite(P_LED, 1);
		else
			digitalWrite(P_LED, 0);
		if(elapsed >= 180*1000UL) {
			setMotionOff();
			digitalWrite(P_LED, 0);
			return;
		}
		break;

	case MotionRepeat0:
		if(elapsed >= motionDur)
			setTargets(repeatMotions[1], motionDur, MotionRepeat1);
		break;

	case MotionRepeat1:
		if(elapsed >= motionDur)
			setTargets(repeatMotions[0], motionDur, MotionRepeat0);
		break;

	case MotionNeutral0:
		if(elapsed >= motionDur) {
			static const int8_t motion[] = {0,0,INV,INV,INV,INV,0,0};
			setTargets(motion, motionDur, MotionNeutral1);
		}
		break;

	case MotionNeutral1:
		if(elapsed >= motionDur) {
			setMotionOff();
			return;
		}
		break;

	default:
		break;
	}
	setTargetLoop();
}

void quadCrawler_setSpeed(uint16_t speed)
{
	motionDur = speed;
	memcpy(&walkCalib256, &walkCalibInit, sizeof(walkCalib256));
}

void quadCrawler_setSpeed(uint16_t speed, int16_t x, int16_t y)
{
	motionDur = speed;
	memcpy(&walkCalib256, &walkCalibInit, sizeof(walkCalib256));

	if(x == 0 || y == 0) return;

	// x/yで進行方向 微調整
	int16_t calib;
	switch(cur_com) {
	case COM_RW:
		x = -x;
	case COM_FW:
		calib = x*256L*2/y;
		if(calib >  256) calib =  256;
		if(calib < -256) calib = -256;
		if(calib>=0)
			walkCalib256.RF = walkCalib256.RR = 256 - calib;
		else
			walkCalib256.LF = walkCalib256.LR = 256 + calib;
		break;

	case COM_RIGHT:
		y = -y;
	case COM_LEFT:
		calib = y*256L*2/x;
		if(calib >  256) calib =  256;
		if(calib < -256) calib = -256;
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
		if(motionState != MotionNeutral0 && motionState != MotionNeutral1 && motionState != MotionOff)
			motionNeutral();
		break;
									//	RF				LF				RR				LR
	case COM_FW: {
		static const s_walk walk[2] = {	{0,			0,			-C_FB2,		-C_FB2},
										{+C_FB2,	+C_FB2,		+C_FB2,		+C_FB2} };	motionWalk(walk);	break; }
	case COM_RW: {
		static const s_walk walk[2] = {	{+C_FB2,	+C_FB2,		0,			0},
										{-C_FB2,	-C_FB2,		-C_FB2,		-C_FB2} };	motionWalk(walk);	break; }
	case COM_LEFT: {
		static const s_walk walk[2] = {	{-C_LR2,	0,			+C_LR2,		0},
										{+C_LR2,	-C_LR2,		-C_LR2,		+C_LR2} };	motionWalk(walk);	break; }
	case COM_RIGHT: {
		static const s_walk walk[2] = {	{0,			-C_LR2,		0,			+C_LR2},
										{-C_LR2,	+C_LR2,		+C_LR2,		-C_LR2} };	motionWalk(walk);	break; }
	case COM_CCW: {
		static const s_walk walk[2] = {	{-C_CW,			+C_CW,			-C_CW,			+C_CW},
										{+C_CW*2,		-C_CW*2,		+C_CW*2,		-C_CW*2} };		motionWalk(walk);	break; }
	case COM_CW: {
		static const s_walk walk[2] = {	{+C_CW,			-C_CW,			+C_CW,			-C_CW},
										{-C_CW*2,		+C_CW*2,		-C_CW*2,		+C_CW*2} };		motionWalk(walk);	break; }

										//	RF			RR			LF			LR
	case COM_NEUTRAL: {
		static const int8_t motion[] =		{0,0,		0,0,		0,0,		0,0};		setTargets(motion,speed,MotionPose);	break; }
	case COM_ALL_UP: {
		static const int8_t motion[] =		{K_U, INV,	K_U, INV,	K_U, INV,	K_U, INV};	setTargets(motion,speed,MotionPose);	break; }
	case COM_ALL_DOWN: {
		static const int8_t motion[] =		{K_D2,INV,	K_D2,INV,	K_D2,INV,	K_D2,INV};	setTargets(motion,speed,MotionPose);	break; }
	case COM_T_DOWN: {
		static const int8_t motion[] =		{K_D, INV,	K_D2,INV,	K_D, INV,	K_D2,INV};	setTargets(motion,speed,MotionPose);	break; }
	case COM_H_DOWN: {
		static const int8_t motion[] =		{K_D2,INV,	K_D, INV,	K_D2,INV,	K_D, INV};	setTargets(motion,speed,MotionPose);	break; }
	case COM_L_DOWN: {
		static const int8_t motion[] =		{K_D, INV,	K_D, INV,	K_D2,INV,	K_D2,INV};	setTargets(motion,speed,MotionPose);	break; }
	case COM_R_DOWN: {
		static const int8_t motion[] =		{K_D2,INV,	K_D2,INV,	K_D, INV,	K_D, INV};	setTargets(motion,speed,MotionPose);	break; }

	case COM_T_UPDOWN: {
		static const int8_t motion[2][8] = {{K_D, INV,	K_D, INV,	K_D, INV,	K_D, INV},
											{K_D, INV,	K_D2,INV,	K_D, INV,	K_D2,INV}};	motionRepeat(motion,speed);	break; }
	case COM_L_R_UP: {
		static const int8_t motion[2][8] = {{K_D2,INV,	K_D2,INV,	K_D, INV,	K_D, INV},
											{K_D, INV,	K_D, INV,	K_D2,INV,	K_D2,INV}};	motionRepeat(motion,speed);	break; }
	case COM_ALL_UPDOWN: {
		static const int8_t motion[2][8] = {{K_D, INV,	K_D, INV,	K_D, INV,	K_D, INV},
											{K_D2,INV,	K_D2,INV,	K_D2,INV,	K_D2,INV}};	motionRepeat(motion,speed);	break; }
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
	cur_com = COM_POSE;
	return calib[id];
}

//---------------------------------------------------------

extern Adafruit_NeoPixel* _pixels;
void _neoPixels(uint8_t port, int num, int color);

static void colorWipe(uint32_t c)
{
	uint8_t i;
	for (i = 0; i < _pixels->numPixels(); i++)
		_pixels->setPixelColor(i, c);
	_pixels->show();
}

void quadCrawler_colorWipe(uint8_t color)
{
	switch(color) {
	case COLOR_OFF:
		colorWipe(_pixels->Color(0,0,0));
		break;
	case COLOR_RED:
		colorWipe(_pixels->Color(255,0,0));
		break;
	case COLOR_GREEN:
		colorWipe(_pixels->Color(0,255,0));
		break;
	case COLOR_BLUE:
		colorWipe(_pixels->Color(0,0,255));
		break;
	case COLOR_YELLOW:
		colorWipe(_pixels->Color(128,128,0));
		break;
	case COLOR_PURPLE:
		colorWipe(_pixels->Color(128,0,128));
		break;
	case COLOR_LIGHTBLUE:
		colorWipe(_pixels->Color(0,128,128));
		break;
	default:
		break;
	}
}

void quadCrawler_colorRed(uint8_t id)
{
	colorWipe(_pixels->Color(0,0,0));

	_pixels->setPixelColor(id*2+0, _pixels->Color(255,0,0));
	_pixels->setPixelColor(id*2+1, _pixels->Color(255,0,0));
	_pixels->show();
	delay(10);
}

static uint32_t Wheel(byte WheelPos)
{
	WheelPos = 255 - WheelPos;
	if (WheelPos < 85) {
		return _pixels->Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	if (WheelPos < 170) {
		WheelPos -= 85;
		return _pixels->Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
	WheelPos -= 170;
	return _pixels->Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void quadCrawler_rainbow(uint8_t wait)
{
	uint16_t i, j;

	for (j = 0; j < 256; j++) {
		for (i = 0; i < _pixels->numPixels(); i++)
			_pixels->setPixelColor(i, Wheel((i + j) & 255));
		_pixels->show();
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
	delay(100);

	pinMode(P_Motor_EN, OUTPUT);
	digitalWrite(P_Motor_EN, HIGH);

	pinMode(P_Echo, INPUT_PULLUP);
	pinMode(P_Trig, OUTPUT);
	pinMode(P_Bz, OUTPUT);
	pinMode(P_Sw1, INPUT_PULLUP);
	pinMode(P_Sw2, INPUT_PULLUP);
	pinMode(P_Sw3, INPUT_PULLUP);
	pinMode(P_Sw4, INPUT_PULLUP);

	_neoPixels(P_Neopix, 8, 0x000000);

	uint8_t i;
	uint8_t initFlag = 1;
	for(i = 0; i < 8; i++) {
		calib[i] = EEPROM.read(EEPROM_CALIB+i);
		if(calib[i] != -1) initFlag = 0;
	}
	if(initFlag) {
		Serial.println(F("init calib"));
		memset(calib, 0, sizeof(calib));
		for(i = 0; i < 8; i++)
			EEPROM.update(EEPROM_CALIB+i, calib[i]);
	}

	pwm.begin();
	pwm.setPWMFreq(50);  // Analog servos run at ~50 Hz updates
	digitalWrite(P_Motor_EN, LOW);
	setServoInit();
}
