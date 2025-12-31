#include "ppu.h"
#include "cpu.h"
#include "general.h"
#include "memory.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdint.h>

#include "ansicolor.h"


// initPpu()
//   initializes PPU
void initPpu(PPU* ppu){

  ppu->chrrom = malloc(sizeof(uint8_t) * 8192);
  ppu->oam = malloc(sizeof(uint8_t) * 64 * 4);
  ppu->paletteram = calloc(32, sizeof(uint8_t));
  ppu->vram = calloc(0x1400, sizeof(uint8_t));

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
  ppu->data = 0;
  ppu->wregister = 0;
  ppu->vblank = 0;
  ppu->xScroll = 0;
  ppu->yScroll = 0;
  ppu->scanLine = 0;
  ppu->frames = 0;
  ppu->vregister1 = 0;

  ppu->vregister2.courseX = 0;
  ppu->vregister2.courseY = 0;
  ppu->vregister2.fineY = 0;
  ppu->vregister2.nameTableSelect = 0;
  ppu->xregister = 0;

  ppu->bitPlane1 = 0;
  ppu->bitPlane2 = 0;

  

}

// dmaTransfer()
//   used to facilitate the dma transfer between CPU memory and PPU OAM (copies a page of 0xff bytes into oam)
// inputs:
//   bus - machine to act upon
void dmaTransfer(Bus* bus){
  uint16_t addr;

  for(int i = 0; i <= 0xff; ++i){
    /*
    if(i % 4 == 0){
      printf("oam dma ");;
    }*/
    
    addr = (((uint16_t)bus->ppu->oamdma) << 8) | i;
    bus->ppu->oam[i] = readBus(bus, addr);    
   // printf("%x ", bus->ppu->oam[i]);

  }
  //printf("\n");
}

// populatePalette()
//   hard-coding 24-bit rgb values for each of the nes' 64 colours
//   table for nes to 24bit color conversion
void populatePalette(PPU* ppu){
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



}


// drawFramebuffer()
//   draws Framebuffer to background layer in sdl 
//   also converts the nes colour palette to rgb values when reading from the frame buffer
void drawFrameBuffer(PPU* ppu, SDL_Renderer* renderer, SDL_Texture* texture){
  //printf("drawing framebuffer \n");
  uint32_t *pixels;
  int pitch;


  SDL_RenderClear(renderer);
  SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch);

  for(int i = 0; i < WINDOW_HEIGHT; ++i){
    for(int j = 0; j < WINDOW_WIDTH; ++j){
      pixels[(i * WINDOW_WIDTH) + j] = ppu->frameBuffer[i][j];

    }
  }

  SDL_UnlockTexture(texture);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);

}

void printNameTable(Bus* bus){

  printf("-------------------------------- \n");
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
  printf("-------------------------------- \n");

}

int getEightSixteen(PPU* ppu){

  if(getBit(ppu->ctrl, 5) == 0){
    return 0;

  // if bit 5 of the ppuctrl is set, enable 8x16 sprites
  } else if (getBit(ppu->ctrl, 5) != 0){
    return 1;
    
  }

}


// spriteEvaluation()
//    performs a sprite evaluation on the PPU's OAM
// input:
//   ppu - ppu to function on
//   eightSixteenSpriteFlag - whether the game is an 8x8 or 8x16 sprite game
// output:
//   oamIndices - output array of oam indices
// return:
//   amount of sprites found on the current scanline
int spriteEvaluation(PPU* ppu, uint8_t* oamIndices, int eightSixteenSpriteFlag){
  int spriteEvalCounter = 0;


  for(uint16_t i = 0; i < 256; i = i + 4){

    // ppu->oam[i] gets the Y coordinate of the tile
    
    if(eightSixteenSpriteFlag == 0){
      if(ppu->oam[i] <= ppu->scanLine && ppu->oam[i] + 7 >= ppu->scanLine){
        oamIndices[spriteEvalCounter] = (uint8_t) i;
        spriteEvalCounter++;
      }
    } else if(eightSixteenSpriteFlag == 1){
      if(ppu->oam[i] <= ppu->scanLine && ppu->oam[i] + 15 >= ppu->scanLine){
        oamIndices[spriteEvalCounter] = i;
        spriteEvalCounter++;
      }
    }

    if(spriteEvalCounter >= 7){
      if(spriteEvalCounter == 7){
        continue;
      }

      if(spriteEvalCounter >= 8){
        ppu->status = setBit(ppu->status, 5);
        spriteEvalCounter = 7;
      }
      break;
    }

  }

  return spriteEvalCounter;


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
  uint16_t patternTableIndice1;
  uint16_t patternTableIndice2;
  uint8_t bitPlane1;
  uint8_t bitPlane2;
  uint8_t bit1;
  uint8_t bit2;
  uint16_t bit1_16;
  uint16_t bit2_16;
  uint8_t bitsCombined;
  uint8_t pixelValue1;
  uint8_t pixelValue2;
  uint8_t currentAttributeData;
  struct VComponent vcomp;
  uint8_t pixelValueFinal;
  uint16_t patternTableOffset;
  uint8_t spritePaletteIndex;
  uint8_t bitsCombinedBackground;
  int spriteEvalCounter = 0;
  int eightSixteenSpriteFlag;
  uint16_t spritePatternTableOffset;
  uint32_t thirtytwobitPixelColour;
  uint16_t spritePatternTableIndice;
  int pixelY;
  uint8_t oamIndices[8];
  uint8_t tempPalette[4]; 
  uint16_t tempV2;
  uint8_t tileNumberOfTopSprite;

  if(getBit(ppu->ctrl, 4) == 0){
    patternTableOffset = 0;
  } else if (getBit(ppu->ctrl, 4) != 0){
    patternTableOffset = 0x1000;
  }

  if(getBit(ppu->ctrl, 3) == 0){
      spritePatternTableOffset = 0;
  } else if(getBit(ppu->ctrl, 3) != 0){
      spritePatternTableOffset = 0x1000;
  }

  eightSixteenSpriteFlag = getEightSixteen(ppu);


  // Sprite Evaluation
  //   Does a linear search through the oam, find 8 sprites on the current scanline that are going to be drawn and
  //   stores the found oam indices into an array
  spriteEvalCounter = spriteEvaluation(ppu, oamIndices, eightSixteenSpriteFlag);




  for(int i = 0; i < WINDOW_WIDTH; ++i){



    // if sprite rendering is enabled
    if(getBit(ppu->mask, 3) == 0b1000){

      // every tile, fetch the two bitplanes from the patterntable two tiles ahead, to keep the buffer filled
      if(i != 0 && i % 8 == 0){ 
        fillTempV(&tempV2, ppu->vregister2);

        patternTableIndice = readPpuBus(ppu, 0x2000 + tempV2);
        // fetch lsb and msb bitplanes and fill shift register buffers
        ppu->bitPlane1 = ppu->bitPlane1 | readPpuBus(ppu, patternTableOffset + (patternTableIndice << 4) + ppu->vregister2.fineY);
        ppu->bitPlane2 = ppu->bitPlane2 | readPpuBus(ppu, patternTableOffset + (patternTableIndice << 4) + ppu->vregister2.fineY + 8);

        attributeTableByte = readPpuBus(ppu, 0x23c0 | (tempV2 & 0x0c00) | ((tempV2 >> 4) & 0x38) | ((tempV2 >> 2) & 0x07));

        // finds which quadrant V resides in and returns the appropriate 2-bit number from the byte
        attributeTableByte = findAndReturnAttributeByte(tempV2, attributeTableByte);

        // copies it 8 times into the attribute table buffer (shift register)
        for(int k = 0; k < 8; ++k){
          if(getBit(attributeTableByte, 0) == 1){
            ppu->attributeData1 = setBit16bit(ppu->attributeData1, k);
          } else if(getBit(attributeTableByte, 0) == 0){
            ppu->attributeData1 = clearBit16bit(ppu->attributeData1, k);
          }

          if(getBit(attributeTableByte, 1) == 0b10){
            ppu->attributeData2 = setBit16bit(ppu->attributeData2, k);
          } else if(getBit(attributeTableByte, 1) == 0){
            ppu->attributeData2 = clearBit16bit(ppu->attributeData2, k);
          }

        }
        incrementCourseX(ppu);
      }

      // fetch data from shift registers for the current pixel
      currentAttributeData = (getBitFromLeft16bit(ppu->attributeData1, ppu->xregister) >> 15);
      currentAttributeData = currentAttributeData | (getBitFromLeft16bit(ppu->attributeData2, ppu->xregister) >> 14);

      // $3f00 is hard-wired to be the backdrop color
      tempPalette[0] = readPpuBus(ppu, 0x3f00 + 0);
      tempPalette[1] = readPpuBus(ppu, 0x3f00 + 1 + (currentAttributeData) * 4);
      tempPalette[2] = readPpuBus(ppu, 0x3f00 + 2 + (currentAttributeData) * 4);
      tempPalette[3] = readPpuBus(ppu, 0x3f00 + 3 + (currentAttributeData) * 4);


      // shift the shift registers
      ppu->attributeData1 = ppu->attributeData1 << 1;
      ppu->attributeData2 = ppu->attributeData2 << 1;


      // using data from the shift register, get the two bits from the bitplanes from the end of the shift registers
      bit1_16 = getBitFromLeft16bit(ppu->bitPlane1, ppu->xregister);
      bit2_16 = getBitFromLeft16bit(ppu->bitPlane2, ppu->xregister);
      bit1_16 = bit1_16 >> findBit16bit(bit1_16);
      bit2_16 = bit2_16 >> findBit16bit(bit2_16);
      bit2_16 = bit2_16 << 1;
      bitsCombined = bit1_16 | bit2_16;

      // find 24Bit rgb value and set the pixel value to this
      ppu->frameBuffer[ppu->scanLine][i] = ppu->palette[tempPalette[bitsCombined]];

      ppu->bitPlane1 = ppu->bitPlane1 << 1;
      ppu->bitPlane2 = ppu->bitPlane2 << 1;


    // this is kept for later when checking for a sprite zero hit
      bitsCombinedBackground = bitsCombined;
    } else if(getBit(ppu->mask, 3) == 0){
      ppu->frameBuffer[ppu->scanLine][i] = ppu->palette[tempPalette[0]];
    }


   

    
    // Second, now iterate through the amount of sprites that were found from Sprite Evaluation and display them if the beam resides in it's X coordinate
    for(int j = 0; j < spriteEvalCounter; ++j){

      // if rendering 8x16 sprites, fetch the spriteOffset from byte 1 of the OAM as opposed to bit 5 of PPUCTRL
      if(eightSixteenSpriteFlag == 1){
        if(getBit(ppu->oam[oamIndices[j] + 1], 0) == 0){
          spritePatternTableOffset = 0;
        } else if(getBit(ppu->oam[oamIndices[j] + 1], 0) == 1){
          spritePatternTableOffset = 0x1000;
        }
      }

      // + 3 because this gets the X coordinate of the tile
      // if the beam is within the boundaries of foreground tile, draw the pixel
      if(ppu->oam[oamIndices[j] + 3] <= i && ppu->oam[oamIndices[j] + 3] + 8 >= i){
 

       if(eightSixteenSpriteFlag == 0){


          // if the sprite is vertically mirrored or not
          if(getBit(ppu->oam[oamIndices[j] + 2], 7) == 0){ 

          // oamIndices[j] + 1 because this is where the patterntable index resides in
            bitPlane1 = readPpuBus(ppu, (spritePatternTableOffset + (((uint16_t) ppu->oam[oamIndices[j] + 1]) << 4) + ppu->scanLine - ppu->oam[oamIndices[j]]));
            bitPlane2 = readPpuBus(ppu, (spritePatternTableOffset + (((uint16_t) ppu->oam[oamIndices[j] + 1]) << 4) + ppu->scanLine - ppu->oam[oamIndices[j]] + 8));    
          } else if(getBit(ppu->oam[oamIndices[j] + 2], 7) == 0b10000000) {
            bitPlane1 = readPpuBus(ppu, (spritePatternTableOffset + (((uint16_t) ppu->oam[oamIndices[j] + 1]) << 4) + (7 - (ppu->scanLine - ppu->oam[oamIndices[j]]))));
            bitPlane2 = readPpuBus(ppu, (spritePatternTableOffset + (((uint16_t) ppu->oam[oamIndices[j] + 1]) << 4) + (7 - (ppu->scanLine - ppu->oam[oamIndices[j]]) + 8)));    
          }

        } else if(eightSixteenSpriteFlag == 1){


          // if the sprite is vertically mirrored or not
          if(getBit(ppu->oam[oamIndices[j] + 2], 7) == 0){ 
            
            
            if((ppu->scanLine - ppu->oam[oamIndices[j]]) <= 7){
              bitPlane1 = readPpuBus(ppu, (spritePatternTableOffset + (((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) >> 0) << 4) + ppu->scanLine - ppu->oam[oamIndices[j]]));
              bitPlane2 = readPpuBus(ppu, (spritePatternTableOffset + (((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) >> 0) << 4) + (ppu->scanLine - ppu->oam[oamIndices[j]]) + 8));    
            } else if((ppu->scanLine - ppu->oam[oamIndices[j]]) > 7){
              bitPlane1 = readPpuBus(ppu, (spritePatternTableOffset + ((((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) >> 0) + 1) << 4) + (ppu->scanLine - ppu->oam[oamIndices[j]]) - 8));
              bitPlane2 = readPpuBus(ppu, (spritePatternTableOffset + ((((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) >> 0) + 1) << 4) + (ppu->scanLine - ppu->oam[oamIndices[j]])));    
            }

          } else if(getBit(ppu->oam[oamIndices[j] + 2], 7) == 0b10000000) {

            if((ppu->scanLine - ppu->oam[oamIndices[j]]) <= 7){
              bitPlane1 = readPpuBus(ppu, (spritePatternTableOffset + ((((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) >> 0) + 1) << 4) + (7 - (ppu->scanLine - ppu->oam[oamIndices[j]]))));
              bitPlane2 = readPpuBus(ppu, (spritePatternTableOffset + ((((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) >> 0) + 1) << 4) + (7 - (ppu->scanLine - ppu->oam[oamIndices[j]] + 8))));      
            } else if((ppu->scanLine - ppu->oam[oamIndices[j]]) > 7){
              bitPlane1 = readPpuBus(ppu, (spritePatternTableOffset + (((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) >> 0) << 4) + (7 - (ppu->scanLine - ppu->oam[oamIndices[j]] - 8))));
              bitPlane2 = readPpuBus(ppu, (spritePatternTableOffset + (((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) >> 0) << 4) + (7 - (ppu->scanLine - ppu->oam[oamIndices[j]]))));    
            }

          }


        }
        
        // checks to see if the sprite horizontal mirroring bit is set
        if(getBit(ppu->oam[oamIndices[j] + 2], 6) == 0b01000000){
          bit1 = getBit(bitPlane1, i - ppu->oam[oamIndices[j] + 3]);
          bit2 = getBit(bitPlane2, i - ppu->oam[oamIndices[j] + 3]);
          bit1 = bit1 >> (i - ppu->oam[oamIndices[j] + 3]);
          bit2 = bit2 >> (i - ppu->oam[oamIndices[j] + 3]);
        } else {
          bit1 = getBitFromLeft(bitPlane1, i - ppu->oam[oamIndices[j] + 3]);
          bit2 = getBitFromLeft(bitPlane2, i - ppu->oam[oamIndices[j] + 3]);
          bit1 = bit1 >> findBit(bit1);
          bit2 = bit2 >> findBit(bit2);
        }



        bit2 = bit2 << 1;
        bitsCombined = bit1 | bit2;

        // if the colour isn't a transparency pixel and sprite rendering is enabled, draw the pixel
        if(bitsCombined != 0 && getBit(ppu->mask, 4) == 0b10000){

          // if the background isn't transparent or the sprite is behind the backgrond, draw the pixel
          if(bitsCombinedBackground == 0 || getBit(ppu->oam[oamIndices[j] + 2], 5) == 0){
          // fetch sprite palette index from oam memory and set palette
            spritePaletteIndex = getBit(ppu->oam[oamIndices[j] + 2], 0);
            spritePaletteIndex = spritePaletteIndex | getBit(ppu->oam[oamIndices[j] + 2], 1);

            tempPalette[0] = readPpuBus(ppu, 0x3f10 + 0 + (spritePaletteIndex * 4));
            tempPalette[1] = readPpuBus(ppu, 0x3f10 + 1 + (spritePaletteIndex * 4));
            tempPalette[2] = readPpuBus(ppu, 0x3f10 + 2 + (spritePaletteIndex * 4));
            tempPalette[3] = readPpuBus(ppu, 0x3f10 + 3 + (spritePaletteIndex * 4));

            ppu->frameBuffer[ppu->scanLine][i] = ppu->palette[tempPalette[bitsCombined]];
          }

         // sprite zero hit detection
          if(oamIndices[j] == 0 && bitsCombinedBackground != 0 && getBit(ppu->mask, 3) != 0 && getBit(ppu->status, 6) == 0 && i != 255){
            ppu->status = setBit(ppu->status, 6);
          }
        }


        // sprite priority implementation
        //   break if the the current pixel's sprite is not transparent, not drawing any further pixels ontop of it and ending iterating 
        //   through the sprites.
        if(bitsCombined != 0){
          break;

        }
    }
  }
  
  }

  // hori(v) = hori(t)
  ppu->vregister2.courseX = ppu->tregister.courseX;
  ppu->vregister2.nameTableSelect = getBit(ppu->vregister2.nameTableSelect, 1) | getBit(ppu->tregister.nameTableSelect, 0);


  incrementY(ppu);
  
  // gets shift registers ready for next scanline; fetches the first two tiles of the next line
  fetchFirstTwoTiles(ppu, patternTableOffset);
 
 
}


void incrementY(PPU* ppu){
  if(ppu->vregister2.fineY < 7){
    ppu->vregister2.fineY++;
  } else {
    ppu->vregister2.fineY = 0;
    if(ppu->vregister2.courseY == 29){
      ppu->vregister2.courseY = 0;
      ppu->vregister2.nameTableSelect = getBit(ppu->vregister2.nameTableSelect, 1) ^ 0b10 | getBit(ppu->vregister2.nameTableSelect, 0);
  
    } else if(ppu->vregister2.courseY == 31){
      ppu->vregister2.courseY = 0;

    } else {
      ppu->vregister2.courseY++;

    }

  }
  


}

// fillTempV()
//   populates an unsigned integer with the individual components of a VComponent
//   into their respective bit positions
//  output:
//    tempV2 - unsigned integer to fill
//  input:
//    vreg - a struct VComponent to source the data from 
void fillTempV(uint16_t *tempV, struct VComponent vreg){
    *tempV = 0;
    *tempV = vreg.courseX;
    *tempV = *tempV | (((uint16_t)vreg.courseY) << 5);
    *tempV = *tempV | (((uint32_t) vreg.nameTableSelect) << 10);
}



void incrementCourseX(PPU* ppu){

    if(ppu->vregister2.courseX == 31){
      ppu->vregister2.courseX = 0;
      ppu->vregister2.nameTableSelect = getBit(ppu->vregister2.nameTableSelect, 1) | (getBit(ppu->vregister2.nameTableSelect, 0) ^ 0b01);

    } else {
      ppu->vregister2.courseX++;
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

void fetchFirstTwoTiles(PPU* ppu, uint16_t patternTableOffset){

  
  uint16_t patternTableIndice;
  uint16_t tempV2;
  uint8_t attributeTableByte;

  fillTempV(&tempV2, ppu->vregister2);
  


  // fetch nametable
  patternTableIndice = readPpuBus(ppu, 0x2000 + tempV2);


  // fetch attribute table byte 
  attributeTableByte = readPpuBus(ppu, 0x23c0 | (tempV2 & 0x0c00) | ((tempV2 >> 4) & 0x38) | ((tempV2 >> 2) & 0x07));
  attributeTableByte = findAndReturnAttributeByte(tempV2, attributeTableByte);

  // copies attributetablebyte to 8 positions in attributeData1 and 2
  for(int j = 0; j < 8; ++j){
    if(getBit(attributeTableByte, 0) == 1){
      ppu->attributeData1 = setBit16bit(ppu->attributeData1, j);
    } else if(getBit(attributeTableByte, 0) == 0){
      ppu->attributeData1 = clearBit16bit(ppu->attributeData1, j);
    }

    if(getBit(attributeTableByte, 1) == 0b10){
      ppu->attributeData2 = setBit16bit(ppu->attributeData2, j);
    } else if(getBit(attributeTableByte, 1) == 0){
      ppu->attributeData2 = clearBit16bit(ppu->attributeData2, j);
    }

  }
  ppu->attributeData1 = ppu->attributeData1 << 8;
  ppu->attributeData2 = ppu->attributeData2 << 8;

    // fetch low and high bitplanes of patterntable
  ppu->bitPlane1 = readPpuBus(ppu, (patternTableOffset + (patternTableIndice << 4) + ppu->vregister2.fineY));
  ppu->bitPlane1 = ppu->bitPlane1 << 8;

  ppu->bitPlane2 = readPpuBus(ppu, (patternTableOffset + (patternTableIndice << 4) + ppu->vregister2.fineY + 8));
  ppu->bitPlane2 = ppu->bitPlane2 << 8;

  incrementCourseX(ppu);
  fillTempV(&tempV2, ppu->vregister2);

    // fetch pattern table ID
  patternTableIndice = readPpuBus(ppu, 0x2000 + tempV2);

    // fetch attribute table byte
  attributeTableByte = readPpuBus(ppu, 0x23c0 | (tempV2 & 0x0c00) | ((tempV2 >> 4) & 0x38) | ((tempV2 >> 2) & 0x07));
  attributeTableByte = findAndReturnAttributeByte(tempV2, attributeTableByte);
  for(int j = 0; j < 8; ++j){
    if(getBit(attributeTableByte, 0) == 1){
      ppu->attributeData1 = setBit16bit(ppu->attributeData1, j);
    } else if(getBit(attributeTableByte, 0) == 0){
      ppu->attributeData1 = clearBit16bit(ppu->attributeData1, j);
    }

    if(getBit(attributeTableByte, 1) == 0b10){
      ppu->attributeData2 = setBit16bit(ppu->attributeData2, j);
    } else if(getBit(attributeTableByte, 1) == 0){
      ppu->attributeData2 = clearBit16bit(ppu->attributeData2, j);
    }

  }


    // fetch low and high bitplanes of patterntable
  ppu->bitPlane1 = ppu->bitPlane1 | readPpuBus(ppu, (patternTableOffset + (patternTableIndice << 4) + ppu->vregister2.fineY));
  ppu->bitPlane2 = ppu->bitPlane2 | readPpuBus(ppu, (patternTableOffset + (patternTableIndice << 4) + ppu->vregister2.fineY + 8));


   
  incrementCourseX(ppu);

 


}

void vblankStart(Bus* bus){
  
  //printf("vblank start! \n");
  
  bus->ppu->status = setBit(bus->ppu->status, 7);
  bus->ppu->vblank = 1;

  // checks vblank enable bit
  if(getBit(bus->ppu->ctrl, 7) == 0b10000000){
    nmi(bus->cpu, bus);
  }


}



void vblankEnd(Bus* bus){
  //printf("vblank end! \n");

  // clears vblank flag

  bus->ppu->status = clearBit(bus->ppu->status, 7);

  bus->ppu->vblank = 0;
  bus->ppu->scanLine++;
  bus->ppu->frames++;

  bus->ppu->scanLine = 0;

  // clears sprite 0 flag
  bus->ppu->status = clearBit(bus->ppu->status, 6);

  // clears sprite overflow flag
  bus->ppu->status = clearBit(bus->ppu->status, 5);

  // hori(v) = hori(t)
  // copy all x components from t to v
  bus->ppu->vregister2.courseX = bus->ppu->tregister.courseX;
  bus->ppu->vregister2.nameTableSelect = (bus->ppu->tregister.nameTableSelect & 0b1);
  

  // vert(v) = vert(t)
  // copy all y components from t to v
  bus->ppu->vregister2.courseY = bus->ppu->tregister.courseY;
  bus->ppu->vregister2.fineY = bus->ppu->tregister.fineY;
  bus->ppu->vregister2.nameTableSelect = getBit(bus->ppu->vregister2.nameTableSelect, 0) | getBit(bus->ppu->tregister.nameTableSelect, 1);


  uint16_t patternTableOffset;
  if(getBit(bus->ppu->ctrl, 4) == 0){
    patternTableOffset = 0;
  } else if (getBit(bus->ppu->ctrl, 4) != 0){
    patternTableOffset = 0x1000;
  }
  
  fetchFirstTwoTiles(bus->ppu, patternTableOffset);
 
 

}







// findAndReturnAttributeByte()
//   finds which quadrant the beam resides in, then returns the appropriate 2-bit attribute data for the corresponding
//   quadrant
// inputs
//   x - x coordinate
//   y - y coordinate
//   attributeTableByte - attributeTabledata to select from
// outputs
//   uint8_t - 2 bit attribute data for corresponding quadrant
/*
 * 

     0  |  1
    ----------
     2  |  3


 */

uint8_t findAndReturnAttributeByte(uint16_t tempV2, uint8_t attributeTableByte){

  int quadrant;

  uint8_t courseX = tempV2 & 0x1f;
  uint8_t courseY = ((tempV2 & 0x3e0) >> 5);

  if((courseX & 0b10) == 0){
    if((courseY & 0b10) == 0){
      quadrant = 0;

    } else if((courseY & 0b10) == 0b10) {
       quadrant = 2;

    }

  } else if((courseX & 0b10) == 0b10){
    if((courseY & 0b10) == 0){
      quadrant = 1;

    } else if((courseY & 0b10) == 0b10) {
       quadrant = 3;

    }

  }
  
  uint8_t atb = attributeTableByte;
  if(quadrant == 0){
      atb = atb & 0b11;
  } else if (quadrant == 1){
    atb = atb & 0b1100;
    atb = atb >> 2;
  } else if(quadrant == 2){
    atb = atb & 0b110000;
    atb = atb >> 4;
  } else if(quadrant == 3){
    atb = atb & 0b11000000;
    atb = atb >> 6;
  }


  return atb;

  
}