#include "ppu.h"
#include "general.h"
#include "memory.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include "ansicolor.h"


void initPpu(PPU* ppu){

  ppu->chrrom = malloc(sizeof(uint8_t) * 8192);
  ppu->oam = malloc(sizeof(uint8_t) * 64 * 4);
  ppu->paletteram = malloc(sizeof(uint8_t) * 32);
  ppu->vram = calloc(2048, sizeof(uint8_t));
  ppu->scanlineBuffer = malloc(sizeof(uint32_t) * WINDOW_WIDTH);

  printf("initializing PPU \n");
  ppu->frameBuffer = malloc(sizeof(uint32_t*) * 242);
  for(int i = 0; i < 242; ++i){
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

  ppu->scanLine = 0;
  ppu->frames = 0;
  
  //ppu->ppubus = malloc(sizeof(PPUBus));

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

  ppu->scanLine = 0;
  ppu->frames = 0;
  ppu->vregister1 = 0;
  ppu->vregister2 = 0;
  

}

// parses nametables from an array containing rom contents in ines format
// or from a space in memory
void parseNametables(uint8_t*){
  

}

void parsePatterntables(uint8_t*){
  


}

// populatePalette()
//   hard-coding 24-bit rgb values for each of the nes' 64 colours
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
  SDL_RenderClear(renderer);

  for(int i = 0; i < WINDOW_HEIGHT; ++i){
    for(int j = 0; j < WINDOW_WIDTH; ++j){
      SDL_SetRenderDrawColor(renderer, (ppu->frameBuffer[i][j] & 0xff0000) >> 16, (ppu->frameBuffer[i][j] & 0xff00) >> 8, (ppu->frameBuffer[i][j] & 0xff), 255);
      SDL_RenderDrawPoint(renderer, j, i);

    }
  }
  SDL_RenderPresent(renderer);
  allocateNewFrameBuffer(ppu);

}

void printNameTable(Bus* bus){


  for(int i = 0; i < 0x1f; ++i){
    printf("%x ", i);
    for(int j = 0; j < 0x20; ++j){
      if(readPpuBus(bus->ppu, 0x2000 + j + (32 * i)) == 0x62){
         red(); 
      } else if(readPpuBus(bus->ppu, 0x2000 + j + (32 * i)) != 0x24){
        yellow();
      }
      printf("%x ", readPpuBus(bus->ppu, 0x2000 + j + (32 * i)));
      default_color(); 
    }
    printf("\n");
  }

}



// renderScanline()
//   renders a scanline with the given registers 
//   inputs:
//     ppu - ppu to render a scanline with 
//

void renderScanline(PPU* ppu){
  uint8_t currentIndiceBitPlane1;
  uint8_t currentIndiceBitPlane2;
  int attributeTableQuandrant;
  uint8_t attributeTableByte;
  uint16_t patternTableIndice;
  uint8_t bitPlane1;
  uint8_t bitPlane2;
  uint8_t bit1;
  uint8_t bit2;
  uint8_t bitsCombined;
  uint8_t pixelValue1;
  uint8_t pixelValue2;
  uint8_t pixelValueFinal;
  uint16_t offset;
  uint32_t thirtytwobitPixelColour;
  int pixelY;
  uint8_t tempPalette[4];

  tempPalette[0] = 0x0f;
  tempPalette[1] = 0x2c;
  tempPalette[2] = 0x38;
  tempPalette[3] = 0x12;


   if(getBit(ppu->ctrl, 4) == 0){
     offset = 0;
    } else if (getBit(ppu->ctrl, 4) != 0){
     offset = 0x1000;
    }
    
 
  for(int i = 0; i < WINDOW_WIDTH; ++i){
    // fetch nametable entry
    patternTableIndice = readPpuBus(ppu, (0x2000 + (uint16_t)(i / 8)) + ((int)(ppu->scanLine / 8) * 32));
    
    ppu->vregister2 = (uint16_t)(i / 8) + ((int)(ppu->scanLine / 8) * 32);
   //printf("vregistger2 %x \n", ppu->vregister2);
   //printf("formula %x \n",  0x23c0 | (ppu->vregister2 & 0x0c00) | ((ppu->vregister2 >> 4) & 0x38) | ((ppu->vregister2 >> 2) & 0x07));
    // fetch attributetable byte using formula
    attributeTableByte = readPpuBus(ppu, 0x23c0 | (ppu->vregister2 & 0x0c00) | ((ppu->vregister2 >> 4) & 0x38) | ((ppu->vregister2 >> 2) & 0x07));
    attributeTableQuandrant = getAttributeQuadrant(i, ppu->scanLine);

    if(attributeTableQuandrant == 0){
      attributeTableByte = attributeTableByte & 0b11;
    } else if (attributeTableQuandrant == 1){
      attributeTableByte = attributeTableByte & 0b1100;
      attributeTableByte = attributeTableByte >> 2;
    } else if(attributeTableQuandrant == 2){
      attributeTableByte = attributeTableByte & 0b110000;
      attributeTableByte = attributeTableByte >> 4;
    } else if(attributeTableQuandrant == 3){
      attributeTableByte = attributeTableByte & 0b11000000;
      attributeTableByte = attributeTableByte >> 6;
    }


    tempPalette[0] = readPpuBus(ppu, 0x3f00 + 0 + (attributeTableByte * 4));
    tempPalette[1] = readPpuBus(ppu, 0x3f00 + 1 + (attributeTableByte * 4));
    tempPalette[2] = readPpuBus(ppu, 0x3f00 + 2 + (attributeTableByte * 4));
    tempPalette[3] = readPpuBus(ppu, 0x3f00 + 3 + (attributeTableByte * 4));


    

    // patternTableIndice << 4 because this will yield 0x0ff0 when you left shift 0xff for instance
    //
    // So indice 0x24 will reside at address 0x0240 for instance
    // if ppuctrl bit 3 is 0, then use 0x0000 as base, 
    // else if ppuctrl bit 3 is 1 then use 0x1000 as base
    bitPlane1 = readPpuBus(ppu, (offset + (patternTableIndice << 4) + (ppu->scanLine % 8)));
    bitPlane2 = readPpuBus(ppu, (offset + (patternTableIndice << 4) + (ppu->scanLine % 8) + 8));
    bit1 = getBitFromLeft(bitPlane1, i % 8);
    bit2 = getBitFromLeft(bitPlane2, i % 8);
    bit1 = bit1 >> findBit(bit1);
    bit2 = bit2 >> findBit(bit2);
    bit2 = bit2 << 1;
    bitsCombined = bit1 | bit2;
    thirtytwobitPixelColour = ppu->palette[tempPalette[bitsCombined]];
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

void vblankStart(Bus* bus){
  
  printf("vblank start! \n");
  
  bus->ppu->status = setBit(bus->ppu->status, 7);
  bus->ppu->vblank = 1;
  // checks vblank enable bit
  if(getBit(bus->ppu->ctrl, 7) == 0b10000000){
    nmi(bus->cpu, bus);
  }

  writePpuBus(bus->ppu, bus->ppu->addr, bus->ppu->data);
}



void vblankEnd(Bus* bus){
  printf("vblank end! \n");

  bus->ppu->status = clearBit(bus->ppu->status, 7);
  bus->ppu->vblank = 0;
  bus->ppu->scanLine = 0;
  bus->ppu->frames++;

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
  
  

}

// appendScanline()
//   appends the scanline buffer to the framebuffer 
//   inputs:
//     ppu - ppu to render a scanline with 
//
void appendScanline(PPU* ppu){

  // segfault on ppu->scanLine = 240 occurs here
  for(int i = 0; i < WINDOW_WIDTH; ++i){
    ppu->frameBuffer[ppu->scanLine][i] = ppu->scanlineBuffer[i];
  }

 
}


// getAttributeQuadrant()
//   returns the numbered quadrant that the beam is in
//   with each quadrant being a 32x32 pixel (4x4 tile) mapped onto a 256x240 pixel cartesian system 
// inputs
//   x - x coordinate
//   y - y coordinate
// outputs
//   int - quadrant number of corresponding beam position
/*
 * 

     0  |  1
    ----------
     2  |  3


 */

int getAttributeQuadrant(int x, int y){

  int quadrant;
  
  if(x <= 31){
    if(y <= 31){
      quadrant = 0;
    } else if(y > 31 && y <= 63){
      quadrant = 2;
    } else if(y > 63 && y <= 95){
      quadrant = 0;
    } else if(y > 95 && y <= 127){
      quadrant = 2;
    } else if(y > 127 && y <= 159){
      quadrant = 0;
    } else if(y > 159 && y <= 191){
      quadrant = 2;
    } else if(y > 191 && y <= 223){
      quadrant = 0;
    } else if(y > 223 && y <= 255){
      quadrant = 2;
    }
  } else if(x > 31 && x <= 63){

    if(y <= 31){
      quadrant = 1;
    } else if(y > 31 && y <= 63){
      quadrant = 3;
    } else if(y > 63 && y <= 95){
      quadrant = 1;
    } else if(y > 95 && y <= 127){
      quadrant = 3;
    } else if(y > 127 && y <= 159){
      quadrant = 1;
    } else if(y > 159 && y <= 191){
      quadrant = 3;
    } else if(y > 191 && y <= 223){
      quadrant = 1;
    } else if(y > 223 && y <= 255){
      quadrant = 3;
    }

  } else if(x > 63 && x <= 95){
    if(y <= 31){
      quadrant = 0;
    } else if(y > 31 && y <= 63){
      quadrant = 2;
    } else if(y > 63 && y <= 95){
      quadrant = 0;
    } else if(y > 95 && y <= 127){
      quadrant = 2;
    } else if(y > 127 && y <= 159){
      quadrant = 0;
    } else if(y > 159 && y <= 191){
      quadrant = 2;
    } else if(y > 191 && y <= 223){
      quadrant = 0;
    } else if(y > 223 && y <= 255){
      quadrant = 2;
    }

  } else if(x > 95 && x <= 127){
    if(y <= 31){
      quadrant = 1;
    } else if(y > 31 && y <= 63){
      quadrant = 3;
    } else if(y > 63 && y <= 95){
      quadrant = 1;
    } else if(y > 95 && y <= 127){
      quadrant = 3;
    } else if(y > 127 && y <= 159){
      quadrant = 1;
    } else if(y > 159 && y <= 191){
      quadrant = 3;
    } else if(y > 191 && y <= 223){
      quadrant = 1;
    } else if(y > 223 && y <= 255){
      quadrant = 3;
    }

  } else if(x > 127 && x <= 159){
    if(y <= 31){
      quadrant = 0;
    } else if(y > 31 && y <= 63){
      quadrant = 2;
    } else if(y > 63 && y <= 95){
      quadrant = 0;
    } else if(y > 95 && y <= 127){
      quadrant = 2;
    } else if(y > 127 && y <= 159){
      quadrant = 0;
    } else if(y > 159 && y <= 191){
      quadrant = 2;
    } else if(y > 191 && y <= 223){
      quadrant = 0;
    } else if(y > 223 && y <= 255){
      quadrant = 2;
    }
  } else if(x > 159 && x <= 191){
    if(y <= 31){
      quadrant = 1;
    } else if(y > 31 && y <= 63){
      quadrant = 3;
    } else if(y > 63 && y <= 95){
      quadrant = 1;
    } else if(y > 95 && y <= 127){
      quadrant = 3;
    } else if(y > 127 && y <= 159){
      quadrant = 1;
    } else if(y > 159 && y <= 191){
      quadrant = 3;
    } else if(y > 191 && y <= 223){
      quadrant = 1;
    } else if(y > 223 && y <= 255){
      quadrant = 3;
    }
  } else if(x > 191 && x <= 223){
    if(y <= 31){
      quadrant = 0;
    } else if(y > 31 && y <= 63){
      quadrant = 2;
    } else if(y > 63 && y <= 95){
      quadrant = 0;
    } else if(y > 95 && y <= 127){
      quadrant = 2;
    } else if(y > 127 && y <= 159){
      quadrant = 0;
    } else if(y > 159 && y <= 191){
      quadrant = 2;
    } else if(y > 191 && y <= 223){
      quadrant = 0;
    } else if(y > 223 && y <= 255){
      quadrant = 2;
    }
  } else if(x > 223 && x <= 255){
    if(y <= 31){
      quadrant = 1;
    } else if(y > 31 && y <= 63){
      quadrant = 3;
    } else if(y > 63 && y <= 95){
      quadrant = 1;
    } else if(y > 95 && y <= 127){
      quadrant = 3;
    } else if(y > 127 && y <= 159){
      quadrant = 1;
    } else if(y > 159 && y <= 191){
      quadrant = 3;
    } else if(y > 191 && y <= 223){
      quadrant = 1;
    } else if(y > 223 && y <= 255){
      quadrant = 3;
    }
  }


  return quadrant;

  
}