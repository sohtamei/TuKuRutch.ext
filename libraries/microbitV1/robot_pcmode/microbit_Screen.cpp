/*
  microbit_Screen.cpp - LED Screen library for micro:bit
  (for "Arduino Core for Nordic Semiconductor nRF5 based boards")
  Copyright (c) 2017 Hideaki Tominaga. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "microbit_Screen.h"

#define colCount    5
#define rowCount    5
typedef struct {
  uint8_t c;
  uint8_t r;
} LED_POINT;

static uint8_t enabled = 0;
static uint8_t curPhysRow = 0;
static uint8_t curLEDs[rowCount] = {0};

#if defined(NRF51_SERIES)
#define physColNum  9
#define physRowNum  3

static const uint8_t physCols[physColNum] = {3, 4, 10, 23, 24, 25, 9, 7, 6};
static const uint8_t physRows[physRowNum] = {26, 27, 28};

static const LED_POINT Phys2Logi[physRowNum][physColNum] =
{
  {{0,0},{2,0},{4,0},{4,3},{3,3},{2,3},{1,3},{0,3},{1,2}},
  {{4,2},{0,2},{2,2},{1,0},{3,0},{3,4},{1,4},{0xFF,0xFF},{0xFF,0xFF}},
  {{2,4},{4,4},{0,4},{0,1},{1,1},{2,1},{3,1},{4,1},{3,2}},
};
#elif defined(NRF52_SERIES)
#define physColNum  5
#define physRowNum  5

static const uint8_t physCols[physColNum] = {4, 7, 3, 6, 10};
static const uint8_t physRows[physRowNum] = {21,22,23,24,25};

static const LED_POINT Phys2Logi[physRowNum][physColNum] =
{
};
#endif

static const uint8_t LED_FONT[96][colCount] =
{
  /* 0x20 - 0x2F */
  {B00000, B00000, B00000, B00000, B00000}, // (Space)
  {B00000, B10111, B00000, B00000, B00000}, // !
  {B00000, B00011, B00000, B00011, B00000}, // "
  {B01010, B11111, B01010, B11111, B01010}, // #
  {B01010, B10111, B10101, B11101, B01010}, // $
  {B10011, B01001, B00100, B10010, B11001}, // %
  {B01010, B10101, B10101, B01010, B10000}, // &
  {B00000, B00011, B00000, B00000, B00000}, // '
  {B00000, B01110, B10001, B00000, B00000}, // (
  {B00000, B10001, B01110, B00000, B00000}, // )
  {B00000, B01010, B00100, B01010, B00000}, // *
  {B00000, B00100, B01110, B00100, B00000}, // +
  {B00000, B10000, B01000, B00000, B00000}, // ,
  {B00000, B00100, B00100, B00100, B00000}, // -
  {B00000, B01000, B00000, B00000, B00000}, // .
  {B10000, B01000, B00100, B00010, B00001}, // /
  /* 0x30 - 0x3F */
  {B01110, B10001, B10001, B01110, B00000}, // 0
  {B00000, B10010, B11111, B10000, B00000}, // 1
  {B11001, B10101, B10101, B10010, B00000}, // 2
  {B01001, B10001, B10101, B01011, B00000}, // 3
  {B01100, B01010, B01001, B11111, B01000}, // 4
  {B10111, B10101, B10101, B10101, B01001}, // 5
  {B01000, B10100, B10110, B10101, B01000}, // 6
  {B10001, B01001, B00101, B00011, B00001}, // 7
  {B01010, B10101, B10101, B10101, B01010}, // 8
  {B00010, B10101, B01101, B00101, B00010}, // 9
  {B00000, B01010, B00000, B00000, B00000}, // :
  {B00000, B10000, B01010, B00000, B00000}, // ;
  {B00000, B00100, B01010, B10001, B00000}, // <
  {B00000, B01010, B01010, B01010, B00000}, // =
  {B00000, B10001, B01010, B00100, B00000}, // >
  {B00010, B00001, B10101, B00101, B00010}, // ?
  /* 0x40 - 0x4F */
  {B01110, B10001, B10101, B01001, B01110}, // @
  {B11110, B00101, B00101, B11110, B00000}, // A
  {B11111, B10101, B10101, B01010, B00000}, // B
  {B01110, B10001, B10001, B10001, B00000}, // C
  {B11111, B10001, B10001, B01110, B00000}, // D
  {B11111, B10101, B10101, B10001, B00000}, // E
  {B11111, B00101, B00101, B00001, B00000}, // F
  {B01110, B10001, B10001, B10101, B01100}, // G
  {B11111, B00100, B00100, B11111, B00000}, // H
  {B10001, B11111, B10001, B00000, B00000}, // I
  {B01001, B10001, B10001, B01111, B00001}, // J
  {B11111, B00100, B01010, B10001, B00000}, // K
  {B11111, B10000, B10000, B10000, B00000}, // L
  {B11111, B00010, B00100, B00010, B11111}, // M
  {B11111, B00010, B00100, B01000, B11111}, // N
  {B01110, B10001, B10001, B01110, B00000}, // O
  /* 0x50 - 0x5F */
  {B11111, B00101, B00101, B00010, B00000}, // P
  {B00110, B01001, B11001, B10110, B00000}, // Q
  {B11111, B00101, B00101, B01010, B10000}, // R
  {B10010, B10101, B10101, B01001, B00000}, // S
  {B00001, B00001, B11111, B00001, B00001}, // T
  {B01111, B10000, B10000, B01111, B00000}, // U
  {B00111, B01000, B10000, B01000, B00111}, // V
  {B11111, B01000, B00100, B01000, B11111}, // W
  {B11011, B00100, B00100, B11011, B00000}, // X
  {B00001, B00010, B11100, B00010, B00001}, // Y
  {B11001, B10101, B10011, B10001, B00000}, // Z
  {B00000, B11111, B10001, B10001, B00000}, // [
  {B00001, B00010, B00100, B01000, B10000}, // (Backslash)
  {B00000, B10001, B10001, B11111, B00000}, // ]
  {B00000, B00010, B00001, B00010, B00000}, // ^
  {B10000, B10000, B10000, B10000, B10000}, // _
  /* 0x60 - 0x6F */
  {B00000, B00001, B00010, B00000, B00000}, // `
  {B01100, B10010, B10010, B11110, B10000}, // a
  {B11111, B10100, B10100, B01000, B00000}, // b
  {B01100, B10010, B10010, B10010, B00000}, // c
  {B01000, B10100, B10100, B11111, B00000}, // d
  {B01110, B10101, B10101, B10010, B00000}, // e
  {B00100, B11110, B00101, B00001, B00000}, // f
  {B00010, B10101, B10101, B01111, B00000}, // g
  {B11111, B00100, B00100, B11000, B00000}, // h
  {B00000, B11101, B00000, B00000, B00000}, // i
  {B00000, B10000, B10000, B01101, B00000}, // j
  {B11111, B00100, B01010, B10000, B00000}, // k
  {B00000, B01111, B10000, B10000, B00000}, // l
  {B11110, B00010, B00100, B00010, B11110}, // m
  {B11110, B00010, B00010, B11100, B00000}, // n
  {B01100, B10010, B10010, B01100, B00000}, // o
  /* 0x70 - 0x7F */
  {B11110, B01010, B01010, B00100, B00000}, // p
  {B00100, B01010, B01010, B11110, B00000}, // q
  {B11100, B00010, B00010, B00010, B00000}, // r
  {B10000, B10100, B01010, B00010, B00000}, // s
  {B00000, B01111, B10100, B10100, B10000}, // t
  {B01110, B10000, B10000, B11110, B10000}, // u
  {B00110, B01000, B10000, B01000, B00110}, // v
  {B11110, B10000, B01000, B10000, B11110}, // w
  {B10010, B01100, B01100, B10010, B00000}, // x
  {B10010, B10100, B01000, B00100, B00010}, // y
  {B10010, B11010, B10110, B10010, B00000}, // z
  {B00000, B00100, B11111, B10001, B00000}, // {
  {B00000, B11111, B00000, B00000, B00000}, // |
  {B10001, B11111, B00100, B00000, B00000}, // }
  {B00000, B00100, B00100, B01000, B01000}, // ~
  {B00000, B00000, B00000, B00000, B00000}  // (Del)
};

void microbit_Screen::showData(const uint8_t *DataArray)
{
	// 1ms x 3
	int physR;
	int physC;
	for(physR = 0; physR < physRowNum; physR++) {
		for(physC = 0; physC < physColNum; physC++) {
			LED_POINT logi = Phys2Logi[physR][physC];
			if(logi.c != 0xFF && (DataArray[logi.c] & (1<<logi.r)))
				digitalWrite(physCols[physC], LOW);
			else
				digitalWrite(physCols[physC], HIGH);
		}
		digitalWrite(physRows[physR], HIGH);
		delay(1);
		digitalWrite(physRows[physR], LOW);
	}
	for(physC = 0; physC < physColNum; physC++)
		digitalWrite(physCols[physC], HIGH);
}

void microbit_Screen::show(const uint8_t *DataArray)
{
	if(!enabled) {
		enabled = 1;
		begin();
	}
	memcpy(curLEDs, DataArray, rowCount);
}

void microbit_Screen::loopScreen(void)
{
	if(!enabled) return;

	digitalWrite(physRows[curPhysRow], LOW);
	curPhysRow++;
	if(curPhysRow >= physRowNum) curPhysRow = 0;

	int physC;
	for(physC = 0; physC < physColNum; physC++) {
		LED_POINT logi = Phys2Logi[curPhysRow][physC];
		if(logi.c != 0xFF && (curLEDs[logi.r] & (1<<logi.c)))
			digitalWrite(physCols[physC], LOW);
		else
			digitalWrite(physCols[physC], HIGH);
	}
	digitalWrite(physRows[curPhysRow], HIGH);
}

void microbit_Screen::begin()
{
  uint8_t i;
  for (i = 0; i < physColNum; i++)  pinMode(physCols[i], OUTPUT);
  for (i = 0; i < physRowNum; i++)  pinMode(physRows[i], OUTPUT);
  clearScreen();
  curPhysRow = 0;
}

void microbit_Screen::disable()
{
  uint8_t i;
  for (i = 0; i < physColNum; i++)  pinMode(physCols[i], INPUT);
  for (i = 0; i < physRowNum; i++)  pinMode(physRows[i], INPUT);
  enabled = 0;
}

void microbit_Screen::clearScreen() {
  uint8_t i;
  for (i = 0; i < physColNum; i++)  digitalWrite(physCols[i], HIGH);
  for (i = 0; i < physRowNum; i++)  digitalWrite(physRows[i], LOW);
  memset(curLEDs, 0, rowCount);
}

void microbit_Screen::showString(const String text, const uint32_t interval) {
  if(!enabled) {
    enabled = 1;
    begin();
  }
  clearScreen();

  String dStr;
  if (text.length() == 0)
    dStr = " ";
  else if (text.length() == 1)
    dStr = text;
  else
    dStr = " " + text + " ";

  bool isSingleChar = (dStr.length() == 1);
  uint32_t bufSize = dStr.length() * (colCount + 1) - 1;
  uint8_t* strBuf = new uint8_t[bufSize];

  uint32_t idx = 0;
  for (uint32_t c = 0; c < dStr.length(); c++) {
    uint32_t charidx = dStr.charAt(c);
    if ((charidx < 0x20) || (charidx > 0x7f))
      charidx = 0x00;
    else
      charidx = charidx - 0x20;
    for (uint8_t x = 0; x < colCount; x++) {
      strBuf[idx] = LED_FONT[charidx][x];
      idx++;
    }
    if (c < dStr.length() - 1) {
      strBuf[idx] = 0x00;
      idx++;
    }
  }

  idx = 0;
  do {
    uint32_t tick = millis();
    do {
      showData(&strBuf[idx]);
    } while ((millis() - tick) < interval);
    idx++;
  } while ((idx < (bufSize - colCount - 1)) && !isSingleChar);

  if(isSingleChar) {
    int col;
    int row;
    for(row = 0; row < rowCount; row++) {
      int cols = 0;
      for(col = 0; col < colCount; col++)
        cols = (cols<<1) | ((strBuf[colCount-1-col] >> row) & 1);
      curLEDs[row] = cols;
    }
  } else {
    clearScreen();
  }
  delete[] strBuf;
}
