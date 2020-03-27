// copyright to SohtaMei 2019.

#include <stdint.h>
#include <Arduino.h>
#include <avr/interrupt.h>

#include "analogRemote.h"

static uint8_t port_irrx;
static void (*funcLed)(uint8_t onoff) = NULL;

enum {
	STATE_H_IDLE,
	STATE_H_HDR,
	STATE_H_BIT,
	STATE_H_ANALOG,

	STATE_L_HDR,
	STATE_L_BIT,
	STATE_L_ANALOG,
};
static volatile uint8_t state;
static volatile uint8_t rawCount;
static volatile uint32_t rawData;

static volatile uint32_t last_timer;
static volatile uint32_t last_timer2 = 0;
static volatile uint32_t last_timer3 = 0;


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
#define DUR_H_TIMEOUT_QUADCRAWLER	300UL	// ms	remote error by colorWipe
#define DUR_H_TIMEOUT_REMOCONROBO	110UL	// ms
static uint16_t dur_h_timeout = DUR_H_TIMEOUT_REMOCONROBO;

// analog remote ------------------------------------------------

#define MATCH2(ticks, desired_us) \
	 ( ticks >= (desired_us) - (DUR_T/2) \
	&& ticks <= (desired_us) + (DUR_T/2))

#define DUR_T			350
#define DUR_H_TIMEOUT_A	300UL	// ms

#define Y_CENTER		16
#define X_CENTER		16
#define BIT_SIZE		15

union rData {
	uint16_t data;
	struct {
		unsigned int keys		: 3;	// bit2~0  :
		unsigned int x			: 5;	// bit7~3  :
		unsigned int y			: 5;	// bit12~8 :
		unsigned int ch			: 2;	// bit14~13: (1)chA, (2)chB, (0)chC
	} ana;
};
static union rData rData;
static int updated = 0;

static volatile uint8_t analog_ch = 0;

// REMOTE -------

static void irq_irrx(void)
{
	uint32_t cur_timer = micros();
	uint16_t diff;// = ((cur_timer - last_timer) & ~0x80000000);
	if(cur_timer - last_timer >= 0x10000UL) {
		diff = 0xFFFF;
	} else {
		diff = cur_timer - last_timer;
	}
	last_timer = cur_timer;

	switch(state) {
	case STATE_H_IDLE:	// H_IDLE -> L_HDR
		state = STATE_L_HDR;
		break;
	case STATE_L_HDR:	// L_HDR -> H_HDR/H_ANALOG
		if(MATCH(diff, DUR_L_HDR)) {
			state = STATE_H_HDR;
		} else if(MATCH2(diff, DUR_T*3)) {
			rawData = 0;
			rawCount = 0;
			state = STATE_H_ANALOG;
		} else {
			state = STATE_H_IDLE;
		}
		break;
	case STATE_H_HDR:	// H_HDR -> L_BIT
		state = STATE_H_IDLE;
		if(MATCH(diff, DUR_H_HDR)) {
			rawData = 0;
			rawCount = 0;
			state = STATE_L_BIT;
		} else if(MATCH(diff, DUR_H_RPT)) {
			if(rData.data) {
			// 一旦timeoutし、repeatが来たとき
				last_timer2 = millis();
				updated = REMOTE_YES;
				if(funcLed) funcLed(1);
			}
		}
		break;
	case STATE_L_BIT:	// L_BIT -> H_BIT
		state = STATE_H_IDLE;
		if(MATCH(diff, DUR_L_BIT)) {
			if(rawCount < 32) {
				state = STATE_H_BIT;
			} else if((((rawData>>8)^rawData) & 0x00FF00FF) == 0x00FF00FF) {
				rData.data = (rawData>>16) & 0xFF;
				last_timer2 = millis();
				updated = REMOTE_YES;
				if(funcLed) funcLed(1);
			}
		}
		break;
	case STATE_H_BIT:	// H_BIT -> L_BIT
		state = STATE_L_BIT;
		rawData = (rawData>>1);
		rawCount++;
		if(MATCH(diff, DUR_H_BIT1)) {
			rawData |= 0x80000000UL;
		} else if(MATCH(diff, DUR_H_BIT0)) {
			;
		} else {
			state = STATE_H_IDLE;
		}
		break;
	case STATE_L_ANALOG:
	case STATE_H_ANALOG:
		if(MATCH2(diff, DUR_T)) {
			if(!(rawCount&1))
				rawData = (rawData<<1) | 1;
			rawCount += 1;
		} else if(MATCH2(diff, DUR_T*2)) {
			rawData = (rawData<<1) | 0;
			rawCount += 2;
		} else {
			state = STATE_H_IDLE;
			goto Error;
		}
		if(rawCount < BIT_SIZE*2) {
			if(state == STATE_L_ANALOG)
				state = STATE_H_ANALOG;
			else
				state = STATE_L_ANALOG;
		} else {
			union rData _rData;
			_rData.data = rawData;
			if(!analog_ch) {
				analog_ch = _rData.ana.ch;
			} else if(_rData.ana.ch == analog_ch) {
				rData.data = rawData;
				last_timer2 = millis();
				updated = REMOTE_ANALOG;
				if(funcLed) funcLed(1);
			}
			state = STATE_H_IDLE;
		}
		break;
	}
Error:
	attachInterrupt(digitalPinToInterrupt(port_irrx), irq_irrx, (state >= STATE_L_HDR) ? RISING: FALLING);
}

int analogRemote::checkUpdated(void)
{
	if(last_timer2) {
	// pressed
		uint32_t timeout = (updated == REMOTE_ANALOG) ? DUR_H_TIMEOUT_A : dur_h_timeout;
		if(millis() - last_timer2 < timeout) {
			if(updated) {
				xyKeys = xyLevel = 0;
				if(updated == REMOTE_ANALOG) {
					keys	= rData.ana.keys + BUTTON_A_XY;
					x		= (rData.ana.x - X_CENTER)*16;
					y		= (rData.ana.y - Y_CENTER)*16;
				} else {
					keys = rData.data;
					x = y = 0;
				}
			}
			if(mode_xyKeys != MODE_NORMAL && updated == REMOTE_ANALOG) {
				static const uint8_t ButtonTable[] = {
					XY_RIGHT,
					XY_UP_R,
					0,
					XY_UP,
					0,
					0,
					0,
					XY_UP_L,
					XY_DOWN_R,
					0,
					0,
					0,
					XY_DOWN,
					0,
					XY_DOWN_L,
					XY_LEFT,
				};

				uint16_t lev = abs(x) + abs(y);
				if(lev >= 40) {
					uint8_t index = 0;
					if(x/2 <  y  ) index += 1;
					if(x   <  y/2) index += 2;
					if(x   < -y/2) index += 4;
					if(x/2 < -y  ) index += 8;

					xyKeys = ButtonTable[index];
					switch(xyKeys) {
					case XY_RIGHT:	lev =  x; break;
					case XY_LEFT:	lev = -x; break;
					case XY_UP:		lev =  y; break;
					case XY_DOWN:	lev = -y; break;
					default:		lev = lev/2; break;
					}
					if(lev >= 256) lev = 255;
					xyLevel = lev;
				} else {
					xyKeys = 0;
					xyLevel = 0;
				}
				if(mode_xyKeys == MODE_XYKEYS_MERGE && keys == BUTTON_A_XY)
				    keys = xyKeys;
			}
		} else if(keys) {
		// timeout & pressed->released
			xyKeys = xyLevel = 0;
			keys = x = y = 0;
			updated = REMOTE_YES;
			if(funcLed) funcLed(0);
			last_timer2 = 0;
			last_timer3 = millis();
		}
	} else {
		uint32_t diff = millis() - last_timer3;

		// buttonA押し -> buttonB押しでTOPが欠けREPが来たときbuttonAにならないようclear
		if(rData.data && diff > dur_h_timeout)
			rData.data = 0;

		diff >>= 7;
		if(diff == 0) {
			;
		} else if((diff % 16) == 0) {	// 2048
			if(funcLed) funcLed(1);
		} else if((diff % 16) == 1) {
			if(funcLed) funcLed(0);
		}
	}
	int _updated = updated;
	updated = 0;
	return _updated;
}

int analogRemote::getRemoteCh(void)
{
	return analog_ch;
}

analogRemote::analogRemote(
		uint8_t _mode_xyKeys,
		uint8_t _port_irrx,
		void (*_funcLed)(uint8_t onoff))
{
	mode_xyKeys	= _mode_xyKeys;
	port_irrx	= _port_irrx;
	funcLed		= _funcLed;

	pinMode(port_irrx, INPUT);
	attachInterrupt(digitalPinToInterrupt(port_irrx), irq_irrx, FALLING);
	state = STATE_H_IDLE;
	rData.data = 0;

	if(mode_xyKeys == MODE_NORMAL)
		dur_h_timeout = DUR_H_TIMEOUT_REMOCONROBO;
	else
		dur_h_timeout = DUR_H_TIMEOUT_QUADCRAWLER;
}
