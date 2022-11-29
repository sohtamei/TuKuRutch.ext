/* copyright (C) 2020 Sohta. */

#include <stdint.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_NeoPixel.h>
#include "Adafruit_MCP23017.h"
#include <Wire.h>
#include <Preferences.h>
#include "QuadCrawlerAI.h"

// ポート定義

#define P_Echo			12	// P_Echo Pin
#define P_Trig			15	// Trigger Pin

#define P_Bz			2	// Bzzer Pin
#define PE_SW			1
#define PE_LED			0
#define PE_Motor_EN		2	// Surbo Moter Drive Enable Pin

#define P_Neopix	14

#define P_SCL		27
#define P_SDA		26

static Preferences preferencesQC;
static int8_t calib[8] = {0,0,0,0,0,0,0,0};

Adafruit_MCP23017 mcp;

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
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
#define C_Fmin	(-70)	// frontLeg-front
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

	MotionVmd,
};

static uint8_t cur_com = COM_STOP;

static uint8_t  motionState = MotionOff;
static uint32_t motionStart = 0;
static uint16_t motionDur = quadCrawler_fast;

#define VMD_FRAME_SIZE 10
#define VMD_FRAME_MAX  200
static uint8_t  motionVmdBuf[VMD_FRAME_MAX][VMD_FRAME_SIZE];
static uint16_t motionVmdNum = 0;
static uint16_t motionVmdIndex = 0;

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
	uint32_t cur = millis();
	int i;
	for(i = 0; i < 8; i++) {
		if(timeDur[i] == 0) continue;

		int32_t elapsed = (cur - timeStart[i]) & 0x7FFFFFFF;
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

	int32_t elapsed = (millis() - motionStart) & 0x7FFFFFFF;
	switch(motionState) {
	case MotionWalk:
		motionWalkLoop(elapsed);
		break;

	// 固定姿勢: 赤点滅、180秒後にサーボOFF
	case MotionPose:
		if((elapsed >> 8) & 1)      // 256
			quadCrawler_LED(1);
		else
			quadCrawler_LED(0);
		if(elapsed >= 180*1000UL) {
			setMotionOff();
			quadCrawler_LED(0);
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

	case MotionVmd: {
		if(elapsed < (GetL16(motionVmdBuf[motionVmdIndex]+0) * 1000UL) / 30)
			break;

		motionVmdIndex++;
		if(motionVmdIndex >= motionVmdNum) {
			motionNeutral();
			break;
		}
		int speed = ((GetL16(motionVmdBuf[motionVmdIndex]+0) - GetL16(motionVmdBuf[motionVmdIndex-1]+0)) * 1000UL) / 30;
		int8_t* motion = (int8_t*)motionVmdBuf[motionVmdIndex]+2;
		for(int i = 0; i < 8; i++) {
			if(motion[i] != INV)
				setTarget(i, motion[i], speed);
		}
	//	Serial.printf("%d %d %d %d %d %d %d %d\n", motion[0], motion[1], motion[2], motion[3], motion[4], motion[5], motion[6], motion[7]);
		break;
	  }
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
									//	RF			LF			RR			LR
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
		static const s_walk walk[2] = {	{-C_CW,		+C_CW,		-C_CW,		+C_CW},
										{+C_CW*2,	-C_CW*2,	+C_CW*2,	-C_CW*2} };	motionWalk(walk);	break; }
	case COM_CW: {
		static const s_walk walk[2] = {	{+C_CW,		-C_CW,		+C_CW,		-C_CW},
										{-C_CW*2,	+C_CW*2,	-C_CW*2,	+C_CW*2} };	motionWalk(walk);	break; }

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
		preferencesQC.putBytes("calib", calib, sizeof(calib));
		return 0;
	}
	preferencesQC.putBytes("calib", calib, sizeof(calib));
	setServo1(id, 0);
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

#define LEDC_BUZZER 15
void quadCrawler_tone(int sound, int ms)
{
	ledcWriteTone(LEDC_BUZZER, sound);
	delay(ms);
	ledcWriteTone(LEDC_BUZZER, 0);
}

void quadCrawler_init(void)
{
	Wire.begin(P_SDA,P_SCL);
	delay(100);

    mcp.begin();
	mcp.pinMode(PE_Motor_EN, OUTPUT);
	mcp.digitalWrite(PE_Motor_EN, HIGH);
	mcp.pinMode(PE_LED, OUTPUT);
	mcp.digitalWrite(PE_LED, LOW);
	mcp.pinMode(PE_SW, INPUT);

	pinMode(P_Echo, INPUT_PULLUP);
	pinMode(P_Trig, OUTPUT);

	ledcSetup(LEDC_BUZZER, 5000/*Hz*/, 13/*bit*/);
	ledcAttachPin(P_Bz, LEDC_BUZZER);

	strip.begin();
	strip.show(); // Initialize all pixels to 'off'

	preferencesQC.begin("QuadCerawler", false);
	if(preferencesQC.getBytes("calib", calib, sizeof(calib)) < sizeof(calib)) {
		Serial.println("init calib");
		memset(calib, 0, sizeof(calib));
		preferencesQC.putBytes("calib", calib, sizeof(calib));
	}

	pwm.begin();
	pwm.setPWMFreq(50);  // Analog servos run at ~50 Hz updates
	mcp.digitalWrite(PE_Motor_EN, LOW);
	setServoInit();
}

void quadCrawler_LED(uint8_t com)
{
	mcp.digitalWrite(PE_LED,com); 
}

int  quadCrawler_SW(void)
{
	return mcp.digitalRead(PE_SW);
}

void quadCrawler_digitalWrite(uint8_t pin, uint8_t data)
{
	mcp.pinMode(pin+8, OUTPUT);
	mcp.digitalWrite(pin+8, data);
}

uint8_t quadCrawler_digitalRead(uint8_t pin)
{
	mcp.pinMode(pin+8, INPUT);
	return mcp.digitalRead(pin+8);
}

void quadCrawler_setMotion(uint8_t* buf, int size)
{
	cur_com = COM_POSE;
	motionVmdNum = size/VMD_FRAME_SIZE;
	if(motionVmdNum > VMD_FRAME_MAX) motionVmdNum = VMD_FRAME_MAX;
	memcpy(motionVmdBuf, buf, motionVmdNum*VMD_FRAME_SIZE);

	for(int i = 0; i < 8; i++) {
		int deg = (int8_t)motionVmdBuf[0][2+i];
		_set_servo(i, deg);
		servoEnd[i] = deg;
		delay(10);
	}

	motionVmdIndex = 1;
	int speed = ((GetL16(motionVmdBuf[motionVmdIndex]+0) - GetL16(motionVmdBuf[motionVmdIndex-1]+0)) * 1000UL) / 30;

	setTargets((int8_t*)motionVmdBuf[motionVmdIndex]+2, speed, MotionVmd);
}

// Lidar -----------------
#ifdef LIDAR_ENABLED
static struct __attribute__((packed)) _packet {	// 47bytes
	uint8_t header;			// 0x54
	uint8_t ver_len;		// 0x2c = 2+2+3*12+2+2
	uint16_t speed;			// in degrees per second;
	uint16_t startAngle;	// unit: 0.01 degree;
	struct __attribute__((packed)) {
		uint16_t distance;
		uint8_t conficence;
	} points[12];
	uint16_t endAngle;		// unit: 0.01 degree；
	uint16_t timestamp;		// ms, recount if reaching to MAX 30000；
	uint8_t crc8;
} packet;
static uint8_t packetIndex = 0;

#define ROTATE_MAX 550
static struct __attribute__((packed)) _point {
	uint16_t distance;
	uint8_t conficence;
} rotate1[ROTATE_MAX];
static uint16_t lastAngle = 0;
static uint16_t rotateCount = 0;
static uint16_t rotateNum = 0;

static uint8_t RespLidar[4+ROTATE_MAX*3];

void loopLidar()
{
	int16_t c;
	while(Serial2.available()>0) {
		c = Serial2.read();
		((uint8_t*)&packet)[packetIndex++] = c;

		switch(packetIndex) {
		case 1:
		  if(c != 0x54)
		    packetIndex = 0;
		  break;
		case 2:
		  if(c != 0x2c) 
		    packetIndex = 0;
		  break;
		case sizeof(packet):
		  packetIndex = 0;

		  int div = packet.endAngle - packet.startAngle;
		  if(div < 0) div += 36000;
		  div = div / 12;
		  int angle = packet.startAngle;
		  int i;
		  for(i = 0; i < 12; i++) {
			if(angle < lastAngle) {
				memcpy(RespLidar+4, rotate1, sizeof(rotate1));
				rotateNum = rotateCount;
				rotateCount = 0;
		//	  Serial.println(rotateNum);
			}
			if(rotateCount < ROTATE_MAX) {
				memcpy(&(rotate1[rotateCount]), &(packet.points[i]), 3);
				rotateCount++;
			}
			lastAngle = angle;

			angle += div;
			if(angle >= 36000) angle -= 36000;
		  }
		  break;
		}
	}
}

void _write(uint8_t* dp, int count);
void _respLidar()
{
  int num = 3*rotateNum;
  RespLidar[0] = 0xff;
  RespLidar[1] = 0x54;
  RespLidar[2] = num&0xff;
  RespLidar[3] = num>>8;
  _write(RespLidar, 4+num);
  rotateNum = 0;
}
#endif // LIDAR_ENABLED
