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

uint8_t setBitFromLeft(uint8_t, uint8_t);
uint8_t clearBitFromLeft(uint8_t, uint8_t);
uint8_t getBitFromLeft(uint8_t, uint8_t);

uint8_t findBit(uint8_t);

uint8_t bitToBitNum(uint8_t);