#pragma once
#include <stdint.h>
#include "memory.h"
#include "general.h"
#include <stdint.h>
#include <SDL2/SDL.h>




enum DeviceTypePPU {CHRRom, CHRRam, VRam};
typedef struct _Mem Mem;
typedef struct _Bus Bus;

typedef struct _PPUBUS {

  Mem* memArr;
  int numOfBlocks;
  


} PPUBus;


typedef struct _PPU {

  // internal ppu registers
  uint8_t ctrl;
  uint8_t mask;
  uint8_t status;
  uint8_t oamaddr;
  uint8_t oamdata;
  uint8_t oamdma;
  uint8_t yScroll;
  uint8_t xScroll;
  uint16_t addr;
  uint8_t data;
  

  // 0 -  vertical arrangement (horizontal mirroring)
  // 1 - horizontal arrangement (vertical mirroring)
  int mirroring;
  
  int scanLine;

  // CHR-ROM cartridge contents
  // 8192 array of bytes 
  uint8_t* chrrom;

  // should be initialized as [256]
  // used for sprites
  uint8_t* oam;

  // used to store nametables (buffer in memory that stores what sprites and which coordinates 
  // they are displayed at)
  uint8_t* vram;

  PPUBus* ppubus;

  // 32 index array of bytes
  uint8_t* paletteram;

  // 0 - 1st read
  // 1 - 2nd read
  // this register is used when demultiplexing PPUSCROLL and PPUADDR
  int wregister; 
  
  // used to hold the ppu address 
  uint16_t vregister1;
  
  // holds the currently drawn nametable address
  uint16_t vregister2;


  // variable to track whether the nes is in vertical blanking or not
  int vblank;
  
  int frames;
  
  // scanline buffer gets appended to the framebuffer at the end of each rendering cycle
  //
  // the framebuffer gets parsed to the screen when a complete frame is drawn
  // (stores the 24-bit RGB value in an array)
  // frameBuffer should be initialized as [WINDOW_HEIGHT][WINDOW_WIDTH]
  uint32_t** frameBuffer;
  uint32_t* scanlineBuffer;

  // nes palette to 24-bit RGB color
  // http://www.romdetectives.com/Wiki/index.php?title=NES_Palette

  uint32_t palette[0x40];

} PPU;

void initPpu(PPU*);
void resetPpu(PPU*, int);

void populatePalette(PPU*);


// render scanline merely parses the internal registers of the PPU and renders a scanline to a memory buffer
void renderScanline(PPU*);
void appendScanline(PPU*);
void vblankToggle(PPU*);
void vblankStart(Bus*);
void vblankEnd(Bus*);

void allocateNewFrameBuffer(PPU*);

void printNameTable(Bus*);

int getAttributeQuadrant(int, int);

void dmaTransfer(Bus*);

void renderScanlineForeground(PPU*);



// draws the completed framebuffer to screen in SDL
void drawFrameBuffer(PPU*, SDL_Renderer*, SDL_Texture*);
