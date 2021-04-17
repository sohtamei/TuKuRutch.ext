/*
  microbit_Screen.h - LED Screen library for micro:bit
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

#ifndef microbit_Screen_h
#define microbit_Screen_h

#include <Arduino.h>

class microbit_Screen {
  private:
    void showData(const uint8_t *DataArray);
    void begin();
    void clearScreen();
  public:
    void disable();
    void show(const uint8_t *DataArray);
    void showString(const String text, const uint32_t interval = 150);
    void loopScreen();
};

#endif
