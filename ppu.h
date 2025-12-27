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

struct VComponent {
  unsigned int courseX : 5;
  unsigned int courseY : 5;
  unsigned int nameTableSelect : 2;
  unsigned int fineY: 3;
};


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
  

  // NOTE: The following registers require an understanding of https://www.nesdev.org/wiki/PPU_scrolling

  // used to hold the ppu address for PPUADDR and PPUDATA
  uint16_t vregister1;
  

  // fineX scroll component
  uint8_t xregister;
  

  // temporary nametable address (offset of top left nametable address)
  struct VComponent tregister;

  // currently drawn nametable address
  struct VComponent vregister2;

  // 16-bit shift registers used for storing pattern table data
  uint16_t bitPlane1;
  uint16_t bitPlane2;

  // shift register for storing attribute data
  // sets of two bits, composed of two sets each
  // |0|0|0|0|S2|S2|S1|S1|
  // each set gets popped off the front (lsb) of the integer
  uint8_t attributeData;

  // variable to track whether the nes is in vertical blanking or not
  int vblank;
  
  int frames;
  
  // scanline buffer gets appended to the framebuffer at the end of each rendering cycle
  //
  // the framebuffer gets parsed to the screen when a complete frame is drawn
  // (stores the 24-bit RGB value in an array)
  // frameBuffer should be initialized as [WINDOW_HEIGHT][WINDOW_WIDTH]
  uint32_t** frameBuffer;

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

uint8_t findAndReturnAttributeByte(int, int, uint8_t);

void dmaTransfer(Bus*);

void renderScanlineForeground(PPU*);

int spriteEvaluation(PPU*, uint8_t*, int);

int getEightSixteen(PPU*);

// draws the completed framebuffer to screen in SDL
void drawFrameBuffer(PPU*, SDL_Renderer*, SDL_Texture*);


void incrementCourseX(PPU*);

void fillTempV(uint16_t*, struct VComponent); 

