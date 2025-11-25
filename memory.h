
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cpu.h"
#include "general.h"
#include "ppu.h"

#define TRUE 1
#define FALSE 0
#define CPUEMU 0

typedef struct _CPU CPU;
typedef struct _PPU PPU;

typedef struct _PPUBUS PPUBus;



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



typedef struct _Bus {
  Mem* memArr;
  uint8_t numOfBlocks;
  CPU* cpu;

  // peripherals on the bus
  PPU* ppu;
  uint8_t controller1;
  uint8_t controller2;
  uint8_t oamdma;

  // mapper number
  int mapper;


} Bus; 

union BusTypes {
  PPUBus* ppubus;
  Bus* bus;

};


void initMemStruct(Mem*, uint64_t, enum DeviceType, int);
void initBus(Bus*, uint16_t);
void clearMem(Mem*);

uint8_t readBus(Bus*, uint16_t);
void writeBus(Bus*, uint16_t, uint8_t);
void mapMemory(Bus*, uint16_t, uint16_t);

int8_t readPpuBus(PPU*, uint16_t);
void writePpuBus(PPU*, uint16_t, uint8_t);


//#endif
