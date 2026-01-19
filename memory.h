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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cpu.h"
#include "general.h"




#define TRUE 1
#define FALSE 0
#define CPUEMU 0

typedef struct _CPU CPU;
typedef struct _PPU PPU;




enum DeviceType {Ram, Rom};

typedef struct _Mem {
  uint8_t* contents;
  enum DeviceType type;  
  int size;
  // startAddr and endAddr are inclusive
  uint16_t startAddr;
  uint16_t endAddr;


  // inuse flag effectively turns it into a dummy 
  // and doesn't allocate any data
  int inuse;

  // flag to show if the block of memory has been mapped to somewhere in memory yet
  int mapped;
} Mem;

// look at https://www.nesdev.org/wiki/MMC1 for understanding of MMC1 Registers

typedef struct _MMC1Register{
  unsigned int reg : 5;
} MMC1Register;

typedef struct _MMC1{
  MMC1Register control;
  MMC1Register chrBank0;
  MMC1Register chrBank1;
  MMC1Register prgBank;
  MMC1Register shiftRegister;

} MMC1;

#include "ppu.h"


typedef struct _controller {

  // variable set whether it has been strobed or not
  // a strobe being a write to $4016 with a 1 then a write to it with zero
  int strobed;


  // button state stored in an unsigned 8bit integer
  // 1 - pressed
  // 0 - not pressed
  //
  // Buttons in this order to the corresponding bit number
  // 0 - A
  // 1 - B
  // 2 - Select
  // 3 - Start
  // 4 - Up
  // 5 - Down
  // 6 - Left
  // 7 - Right
  uint8_t sdlButtons;
  uint8_t latchedButtons;


  // used for NES Zapper (0 if detected, 1 if not detected)
  uint8_t lightSensor;

  // 1 if half pulled, 0 if fully released and fully pulled
  uint8_t triggerPulled;


  int readCount;

} Controller;



typedef struct _Bus {
  Mem* memArr;
  uint8_t numOfBlocks;
  CPU* cpu;


  // peripherals on the bus
  PPU* ppu;
  Controller controller1;
  Controller controller2;
  uint8_t oamdma;

  // mapper number
  int mapper;

  // used for UxROM games
  uint8_t bankSelect;

  // used as the shift register for MMC1 games
  // only 5 bits in length
  MMC1 mmc1;

  int presenceOfPrgRam;




} Bus; 




void initMemStruct(Mem*, uint64_t, enum DeviceType, int);
void initBus(Bus*, uint16_t);
void clearMem(Mem*);

uint8_t readBus(Bus*, uint16_t);
void writeBus(Bus*, uint16_t, uint8_t);
void mapMemory(Bus*, uint16_t, uint16_t);

void initMmc(MMC1*);

uint8_t readPpuBus(PPU*, uint16_t);
void writePpuBus(PPU*, uint16_t, uint8_t);


//#endif
