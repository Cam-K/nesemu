/*

    nesemu, a Nintendo Entertainment System emulator
    Copyright (C) 2026  Cameron Kelly

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.



*/


#pragma once
#include <stdint.h>


// to compile in code meant for the nes emulator, set this to 1
// otherwise 0 for cpu only
#define NESEMU 1

// used to suppress debug output
#define SUPPRESSOUTPUT 1
#define WINDOW_WIDTH 256
#define WINDOW_HEIGHT 240
// general.h
//   includes helper functions that are useful in many places as well as 
//   variables and macros in multiple places


uint8_t setBit(uint8_t, uint8_t);
uint8_t clearBit(uint8_t, uint8_t);
uint8_t getBit(uint8_t, uint8_t);



uint16_t setBit16bit(uint16_t, uint8_t);
uint16_t clearBit16bit(uint16_t, uint8_t);
uint16_t getBit16bit(uint16_t, uint8_t);


uint8_t setBitFromLeft(uint8_t, uint8_t);
uint8_t clearBitFromLeft(uint8_t, uint8_t);
uint8_t getBitFromLeft(uint8_t, uint8_t);

uint8_t findBit(uint8_t);

uint8_t bitToBitNum(uint8_t);

uint8_t shiftRightWithWrap(uint8_t, uint8_t);
uint8_t shiftLeftWithWrap(uint8_t, uint8_t);

uint16_t getBitFromLeft16bit(uint16_t, uint8_t);
uint16_t clearBitFromLeft16bit(uint16_t, uint8_t);
uint16_t setBitFromLeft16bit(uint16_t, uint8_t);
uint16_t findBit16bit(uint16_t);