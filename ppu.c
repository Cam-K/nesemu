#include "ppu.h"
#include "general.h"
#include "memory.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>



void initPpu(PPU* ppu){

  ppu->chrrom = malloc(sizeof(uint8_t) * 8192);
  ppu->oam = malloc(sizeof(uint8_t) * 64 * 4);
  ppu->paletteram = malloc(sizeof(uint8_t) * 32);
  ppu->vram = malloc(sizeof(uint8_t) * 2048);
  ppu->scanlineBuffer = malloc(sizeof(uint32_t) * WINDOW_WIDTH);

  printf("initializing PPU \n");
  ppu->frameBuffer = malloc(sizeof(uint32_t*) * WINDOW_HEIGHT);
  for(int i = 0; i < WINDOW_HEIGHT; ++i){
    ppu->frameBuffer[i] = malloc(sizeof(uint32_t) * WINDOW_WIDTH);
  } 

  
  ppu->ctrl = 0;
  ppu->mask = 0;
  ppu->status = 0;
  ppu->oamaddr = 0;
  ppu->oamdata = 0;
  ppu->scroll = 0;
  ppu->addr = 0;
  ppu->data = 0;
  ppu->oamdma = 0;

//  ppu->ppubus = malloc(sizeof(PPUBus));

}

// powerFlag is used to denote whether the reset is used
// for a power up as opposed to just a reset
void resetPpu(PPU* ppu, int powerFlag){
  ppu->ctrl = 0;
  ppu->mask = 0;
  ppu->status = (getBit(ppu->status, 7) | 0);
  if(powerFlag == 1){
    ppu->oamaddr = 0;
    ppu->addr = 0;
    ppu->status = 0b10100000;
  }
  ppu->scroll = 0;
  ppu->data = 0;
  ppu->wregister = 0;
  ppu->vblank = 0;


}

// parses nametables from an array containing rom contents in ines format
// or from a space in memory
void parseNametables(uint8_t*){
  

}

void parsePatterntables(uint8_t*){
  


}

// hard-coding rgb values for each of the nes' 64 colours
void populatePalette(PPU* ppu){
  printf("Populating palette... \n");
  ppu->palette[0] = 0x7C7C7C;
  ppu->palette[1] = 0x0000FC;
  ppu->palette[2] = 0x0000BC;
  ppu->palette[3] = 0x4428BC;
  ppu->palette[4] = 0x940084;
  ppu->palette[5] = 0xA80020;
  ppu->palette[6] = 0xA81000;
  ppu->palette[7] = 0x881400;
  ppu->palette[8] = 0x503000;
  ppu->palette[9] = 0x007800;
  ppu->palette[10] = 0x006800;
  ppu->palette[11] = 0x005800;
  ppu->palette[12] = 0x004058;
  ppu->palette[13] = 0x000000;
  ppu->palette[14] = 0x000000;
  ppu->palette[15] = 0x000000;
  ppu->palette[16] = 0xBCBCBC;
  ppu->palette[17] = 0x0078F8;
  ppu->palette[18] = 0x0058F8;
  ppu->palette[19] = 0x6844FC;
  ppu->palette[20] = 0xD800CC;
  ppu->palette[21] = 0xE40058;
  ppu->palette[22] = 0xF83800;
  ppu->palette[23] = 0xE45C10;
  ppu->palette[24] = 0xAC7C00;
  ppu->palette[25] = 0x00B800;
  ppu->palette[26] = 0x00A800;
  ppu->palette[27] = 0x00A844;
  ppu->palette[28] = 0x008888;
  ppu->palette[29] = 0x000000;
  ppu->palette[30] = 0x000000;
  ppu->palette[31] = 0x000000;
  ppu->palette[32] = 0xF8F8F8;
  ppu->palette[33] = 0x3CBCFC;
  ppu->palette[34] = 0x6888FC;
  ppu->palette[35] = 0x9878F8;
  ppu->palette[36] = 0xF878F8;
  ppu->palette[37] = 0xF85898;
  ppu->palette[38] = 0xF87858;
  ppu->palette[39] = 0xFCA044;
  ppu->palette[40] = 0xF8B800;
  ppu->palette[41] = 0xB8F818;
  ppu->palette[42] = 0x58D854;
  ppu->palette[43] = 0x58F898;
  ppu->palette[44] = 0x00E8D8;
  ppu->palette[45] = 0x787878;
  ppu->palette[46] = 0x000000;
  ppu->palette[47] = 0x000000;
  ppu->palette[48] = 0xFCFCFC;
  ppu->palette[49] = 0xA4E4FC;
  ppu->palette[50] = 0xB8B8F8;
  ppu->palette[51] = 0xD8B8F8;
  ppu->palette[52] = 0xF8B8F8;
  ppu->palette[53] = 0xF8A4C0;
  ppu->palette[54] = 0xF0D0B0;
  ppu->palette[55] = 0xFCE0A8;
  ppu->palette[56] = 0xF8D878;
  ppu->palette[57] = 0xD8F878;
  ppu->palette[58] = 0xB8F8B8;
  ppu->palette[59] = 0xB8F8D8;
  ppu->palette[60] = 0x00FCFC;
  ppu->palette[61] = 0xF8D8F8;
  ppu->palette[62] = 0x000000;
  ppu->palette[63] = 0x000000;
  printf("palette populated! \n");


}


// draws Framebuffer to background layer in sdl 
// also converts the nes colour palette to rgb values when reading from the frame buffer
void drawFrameBuffer(PPU* ppu, SDL_Renderer* renderer){
  //printf("drawing framebuffer \n");
  for(int i = 0; i < WINDOW_HEIGHT; ++i){
    for(int j = 0; j < WINDOW_WIDTH; ++j){
      SDL_SetRenderDrawColor(renderer, (ppu->frameBuffer[i][j] & 0xff0000) >> 16, (ppu->frameBuffer[i][j] & 0xff00) >> 8, (ppu->frameBuffer[i][j] & 0xff), 255);
      SDL_RenderDrawPoint(renderer, j, i);
    }
  }
  SDL_RenderPresent(renderer);
  allocateNewFrameBuffer(ppu);

}



// renderScanline()
//   renders a scanline with the given registers 
//   inputs:
//     ppu - ppu to render a scanline with 
//
// how this function should work:
// - parse 
void renderScanline(PPU* ppu){
  uint8_t currentIndiceBitPlane1;
  uint8_t currentIndiceBitPlane2;
  uint8_t nameTableIndice;
  int pixelXBitPlane1;
  int pixelXBitPlane2;
  uint8_t pixelValue1;
  uint8_t pixelValue2;
  uint8_t pixelValueFinal;
  uint32_t thirtytwobitPixelColour;
  int pixelY;
  uint8_t tempPalette[4];

  tempPalette[0] = 0x0f;
  tempPalette[1] = 0x30;
  tempPalette[2] = 0x27;
  tempPalette[3] = 0x2a;

  for(int i = 0; i < WINDOW_WIDTH; ++i){
    // fetches patterntable indice that's contained in the nametable
    // i / 8 because it increments 1 for every 8 pixels, which is what we want to do if we are fetching a different 
    // pattern table every 8 pixels (width of nametable entries is 8 pixels)
    nameTableIndice = readPpuBus(ppu, (int)(0x2000 + i / 8));

    pixelXBitPlane1 = i % 8;
    pixelXBitPlane2 = (i % 8) + 8;
    // 
    // fetches pattern table byte low and high byte
    pixelValue1 = getBit(readPpuBus(ppu, nameTableIndice + 0x1000), pixelXBitPlane1);
    pixelValue2 = getBit(readPpuBus(ppu, nameTableIndice + 0x1000 + pixelXBitPlane2), pixelXBitPlane1);

    pixelValueFinal = pixelValue1 >> pixelXBitPlane1;
    pixelValueFinal = pixelValueFinal | (pixelValue2 >> (pixelXBitPlane1 - 1));
    

    // get pallette information for pixel
    thirtytwobitPixelColour = ppu->palette[tempPalette[pixelValueFinal]]; 
    ppu->scanlineBuffer[i] = thirtytwobitPixelColour;
    
    

  }
  
  
  
  
 
}



// allocateFrameBufffer()
//   frees and allocates a new framebuffer
void allocateNewFrameBuffer(PPU* ppu){


  //printf("Allocating new framebuffer \n");
  for(int i = 0; i < WINDOW_HEIGHT; ++i){
    free(ppu->frameBuffer[i]);
  } 

  free(ppu->frameBuffer);
  ppu->frameBuffer = malloc(sizeof(uint32_t*) * WINDOW_HEIGHT);
  for(int i = 0; i < WINDOW_HEIGHT; ++i){
    ppu->frameBuffer[i] = malloc(sizeof(uint32_t) * WINDOW_WIDTH);
  } 


}

void vblankStart(PPU* ppu){
  
  printf("vblank start! \n");
  
  ppu->status = setBit(ppu->status, 7);
  ppu->vblank = 1;

  writePpuBus(ppu, ppu->addr, ppu->data);
}



void vblankEnd(PPU* ppu){
  printf("vblank end! \n");

  ppu->status = clearBit(ppu->status, 7);
  ppu->vblank = 0;

}


// vblank()
//  operations that occur during a vblank are in this function
//   inputs:
//      ppu - ppu to act upon
void vblankToggle(PPU* ppu){


  if(ppu->vblank == 0){
    printf("vblank toggled on! \n");
    ppu->status = setBit(ppu->status, 7);
    ppu->vblank = 1;
  } else if(ppu->vblank == 1){
    printf("vblank toggled off! \n");
    ppu->status = clearBit(ppu->status, 7);
    ppu->vblank = 0;
  }
  
  // latches data at address $2006/$2007 and writes it to the appropriate ppu address
  writePpuBus(ppu, ppu->addr, ppu->data);
  

}

// appendScanline()
//   appends the scanline buffer to the framebuffer 
//   inputs:
//     ppu - ppu to render a scanline with 
//
void appendScanline(PPU* ppu, int scanLine){

  for(int i = 0; i < WINDOW_WIDTH; ++i){
    ppu->frameBuffer[scanLine][i] = ppu->scanlineBuffer[i];
  }

  //free(ppu->scanlineBuffer);
  //ppu->scanlineBuffer = malloc(sizeof(uint32_t) * WINDOW_WIDTH);
}