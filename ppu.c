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


#include "ppu.h"
#include "cpu.h"
#include "general.h"
#include "memory.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdint.h>
#include <stdio.h>




// initPpu()
//   initializes PPU
void initPpu(PPU* ppu, int banks){

  ppu->chrrom = calloc(8192, sizeof(uint8_t));
  ppu->oam = calloc(256, sizeof(uint8_t));
  ppu->paletteram = calloc(32, sizeof(uint8_t));
  ppu->ppubus = calloc(1, sizeof(PPUBus));

  if(banks > 0){
    ppu->ppubus->memArr = calloc(banks, sizeof(Mem));
  } else if(banks == 0){
    ppu->ppubus->memArr = calloc(1, sizeof(Mem));
  }
  ppu->ppubus->numOfBlocks = banks;
  printf("initializing PPU \n");
  ppu->frameBuffer = malloc(sizeof(uint32_t*) * 242);
  
  for(int i = 0; i < 242; ++i){
    ppu->frameBuffer[i] = calloc(WINDOW_WIDTH, sizeof(uint32_t));
  } 

  
  ppu->ctrl = 0;
  ppu->mask = 0;
  ppu->status = 0;
  ppu->oamaddr = 0;
  ppu->oamdata = 0;
  ppu->addr = 0;
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

  ppu->wregister = 0;
  ppu->vblank = 0;
  ppu->xScroll = 0;
  ppu->yScroll = 0;
  ppu->scanLine = 0;
  ppu->frames = 0;

  ppu->xregister = 0;
  ppu->vregister.vreg = 0;
  ppu->tregister.vreg = 0;

  ppu->bitPlane1 = 0;
  ppu->bitPlane2 = 0;
  ppu->data = 0;

  ppu->scanLineSprites = -1;

  ppu->prerenderScanlineFlag = 0;
  ppu->attributeData1 = 0;
  ppu->attributeData2 = 0;

  ppu->hblank = 0;
  ppu->flagChrRam = 0;

  
  

}

// dmaTransfer()
//   used to facilitate the dma transfer between CPU memory and PPU OAM (copies a page of 0xff bytes into oam)
// inputs:
//   bus - machine to act upon
void dmaTransfer(Bus* bus){
  uint16_t addr;

  for(int i = 0; i <= 0xff; ++i){
    addr = (((uint16_t)bus->ppu->oamdma) << 8) | i;
    bus->ppu->oam[i] = readBus(bus, addr);    
  }
}

// populatePalette()
//   hard-coding 24-bit rgb values for each of the nes' 64 colours
//   table for nes to 24bit color conversion
void populatePalette(PPU* ppu){

  
  ppu->palette[0]  = 0x626262;
  ppu->palette[1]  = 0x001fb2;
  ppu->palette[2]  = 0x2404c8;
  ppu->palette[3]  = 0x5200b2;
  ppu->palette[4]  = 0x730076;
  ppu->palette[5]  = 0x800024;
  ppu->palette[6]  = 0x730b00;
  ppu->palette[7]  = 0x522800;
  ppu->palette[8]  = 0x244400;
  ppu->palette[9]  = 0x005700;
  ppu->palette[10] = 0x005c00;
  ppu->palette[11] = 0x005324;
  ppu->palette[12] = 0x003c76;
  ppu->palette[13] = 0x000000;
  ppu->palette[14] = 0x000000;
  ppu->palette[15] = 0x000000;
  
  ppu->palette[16] = 0xababab;
  ppu->palette[17] = 0x0d57ff;
  ppu->palette[18] = 0x4b30ff;
  ppu->palette[19] = 0x8a13ff;
  ppu->palette[20] = 0xbc08d6;
  ppu->palette[21] = 0xd21269;
  ppu->palette[22] = 0xc72e00;
  ppu->palette[23] = 0x9d5400;
  ppu->palette[24] = 0x607b00;
  ppu->palette[25] = 0x209800;
  ppu->palette[26] = 0x00a300;
  ppu->palette[27] = 0x009942;
  ppu->palette[28] = 0x007db4;
  ppu->palette[29] = 0x000000;
  ppu->palette[30] = 0x000000;
  ppu->palette[31] = 0x000000;
  
  ppu->palette[32] = 0xffffff;
  ppu->palette[33] = 0x53aeff;
  ppu->palette[34] = 0x9085ff;
  ppu->palette[35] = 0xd365ff;
  ppu->palette[36] = 0xff57ff;
  ppu->palette[37] = 0xff5dcf;
  ppu->palette[38] = 0xff7757;
  ppu->palette[39] = 0xfa9e00;
  ppu->palette[40] = 0xbdc700;
  ppu->palette[41] = 0x7ae700;
  ppu->palette[42] = 0x43f611;
  ppu->palette[43] = 0x26ef7e;
  ppu->palette[44] = 0x2cd5f6;
  ppu->palette[45] = 0x4e4e4e;
  ppu->palette[46] = 0x000000;
  ppu->palette[47] = 0x000000;
  
  ppu->palette[48] = 0xffffff;
  ppu->palette[49] = 0xb6e1ff;
  ppu->palette[50] = 0xced1ff;
  ppu->palette[51] = 0xe9c3ff;
  ppu->palette[52] = 0xffbcff;
  ppu->palette[53] = 0xffbdf4;
  ppu->palette[54] = 0xffc6c3;
  ppu->palette[55] = 0xffd59a;
  ppu->palette[56] = 0xe9e681;
  ppu->palette[57] = 0xcef481;
  ppu->palette[58] = 0xb6fb9a;
  ppu->palette[59] = 0xa9fac3;
  ppu->palette[60] = 0xa9f0f4;
  ppu->palette[61] = 0xb8b8b8;
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
      
      printf("%x ", readPpuBus(bus->ppu, 0x2000 + j + (32 * i)));

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
  return -1;
}

// copyMmc1()
//   meant to copy a set of MMC1 registers from one struct to another
void copyMmc1(MMC1* src, MMC1* dest){
  dest->chrBank0.reg = src->chrBank0.reg;
  dest->chrBank1.reg = src->chrBank1.reg;
  dest->control.reg = src->control.reg;
  dest->prgBank.reg = src->prgBank.reg;
  
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
      if(ppu->oam[i] <= ppu->scanLineSprites && ppu->oam[i] + 7 >= ppu->scanLineSprites){
        oamIndices[spriteEvalCounter] = (uint8_t) i;
        spriteEvalCounter++;
      }
    } else if(eightSixteenSpriteFlag == 1){
      if(ppu->oam[i] <= ppu->scanLineSprites && ppu->oam[i] + 15 >= ppu->scanLineSprites){
        oamIndices[spriteEvalCounter] = i;
        spriteEvalCounter++;
      }
    }

    if(spriteEvalCounter >= 8){
      if(spriteEvalCounter == 8){
        continue;
      }

      if(spriteEvalCounter >= 9){
        ppu->status = setBit(ppu->status, 5);
        spriteEvalCounter = 8;
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

  uint8_t attributeTableByte;
  uint16_t patternTableIndice;
  uint8_t bitPlane1 = 0;
  uint8_t bitPlane2 = 0;
  uint8_t bit1;
  uint8_t bit2;
  uint16_t bit1_16;
  uint16_t bit2_16;
  uint8_t bitsCombined;
  uint8_t currentAttributeData;
  uint16_t currentAttributeData1;
  uint16_t currentAttributeData2;
  uint16_t patternTableOffset = 0;
  uint8_t spritePaletteIndex;
  uint8_t bitsCombinedBackground = 0;
  int spriteEvalCounter = 0;
  int eightSixteenSpriteFlag;
  uint16_t spritePatternTableOffset = 0;
  uint8_t oamIndices[9];
  uint8_t tempPalette[4]; 
  uint16_t tempV;

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
  //   Sprite evalution does not occur on scanline 0, so we start the sprite scanline at -1 and increment from here.
  //   (no sprites if scanlinesprites == -1)
  if(ppu->scanLineSprites > -1){
    spriteEvalCounter = spriteEvaluation(ppu, oamIndices, eightSixteenSpriteFlag);
  }



  for(int i = 0; i < WINDOW_WIDTH; ++i){


    
    // if background rendering is enabled
    if(getBit(ppu->mask, 3) == 0b1000){
      // every tile, fetch the bitplanes from the nametable a tile ahead, to keep the buffer filled
      if(i != 0 && i % 8 == 0){ 
        fillTempV(&tempV, ppu->vregister.vcomp);
        
        //printf("vreg %x \n", tempV + 0x2000);
        patternTableIndice = readPpuBus(ppu, 0x2000 + tempV);


        // fetch lsb and msb bitplanes and fill shift register buffers
        ppu->bitPlane1 = ppu->bitPlane1 | readPpuBus(ppu, patternTableOffset + (patternTableIndice << 4) + ppu->vregister.vcomp.fineY);
        ppu->bitPlane2 = ppu->bitPlane2 | readPpuBus(ppu, patternTableOffset + (patternTableIndice << 4) + ppu->vregister.vcomp.fineY + 8);

        attributeTableByte = readPpuBus(ppu, 0x23c0 | (tempV & 0x0c00) | ((tempV >> 4) & 0x38) | ((tempV >> 2) & 0x07));

        // finds which quadrant V resides in and returns the appropriate 2-bit number from the byte
        attributeTableByte = findAndReturnAttributeByte(tempV, attributeTableByte);

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
      currentAttributeData1 = 0;
      currentAttributeData1 = (getBitFromLeft16bit(ppu->attributeData1, ppu->xregister));
      currentAttributeData1 = currentAttributeData1 >> findBit16bit(currentAttributeData1);
      
      currentAttributeData2 = 0;
      currentAttributeData2 = (getBitFromLeft16bit(ppu->attributeData2, ppu->xregister));
      currentAttributeData2 = currentAttributeData2 >> findBit16bit(currentAttributeData2);
      currentAttributeData2 = currentAttributeData2 << 1;
      

      currentAttributeData = currentAttributeData1 | currentAttributeData2;

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
      bitsCombinedBackground = 0;
      ppu->frameBuffer[ppu->scanLine][i] = ppu->palette[readPpuBus(ppu, 0x3f00 + 0)];
    }


   

    // if sprite rendering is enabled
    if(getBit(ppu->mask, 4) != 0 && ppu->scanLineSprites > -1){
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
            bitPlane1 = readPpuBus(ppu, (spritePatternTableOffset + (((uint16_t) ppu->oam[oamIndices[j] + 1]) << 4) + ppu->scanLineSprites - ppu->oam[oamIndices[j]]));
            bitPlane2 = readPpuBus(ppu, (spritePatternTableOffset + (((uint16_t) ppu->oam[oamIndices[j] + 1]) << 4) + ppu->scanLineSprites - ppu->oam[oamIndices[j]] + 8));    
          } else if(getBit(ppu->oam[oamIndices[j] + 2], 7) == 0b10000000) {
            bitPlane1 = readPpuBus(ppu, (spritePatternTableOffset + (((uint16_t) ppu->oam[oamIndices[j] + 1]) << 4) + (7 - (ppu->scanLineSprites - ppu->oam[oamIndices[j]]))));
            bitPlane2 = readPpuBus(ppu, (spritePatternTableOffset + (((uint16_t) ppu->oam[oamIndices[j] + 1]) << 4) + (7 - (ppu->scanLineSprites - ppu->oam[oamIndices[j]]) + 8)));    
          }

        } else if(eightSixteenSpriteFlag == 1){


          // if the sprite is vertically mirrored or not
          if(getBit(ppu->oam[oamIndices[j] + 2], 7) == 0){ 
            if((ppu->scanLineSprites - ppu->oam[oamIndices[j]]) <= 7){
              bitPlane1 = readPpuBus(ppu, (spritePatternTableOffset + (((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) >> 0) << 4) + ppu->scanLineSprites - ppu->oam[oamIndices[j]]));
              bitPlane2 = readPpuBus(ppu, (spritePatternTableOffset + (((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) >> 0) << 4) + (ppu->scanLineSprites - ppu->oam[oamIndices[j]]) + 8));    
            } else if((ppu->scanLineSprites - ppu->oam[oamIndices[j]]) > 7){
              bitPlane1 = readPpuBus(ppu, (spritePatternTableOffset + ((((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) >> 0) + 1) << 4) + (ppu->scanLineSprites - ppu->oam[oamIndices[j]]) - 8));
              bitPlane2 = readPpuBus(ppu, (spritePatternTableOffset + ((((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) >> 0) + 1) << 4) + (ppu->scanLineSprites - ppu->oam[oamIndices[j]])));    
            }

          } else if(getBit(ppu->oam[oamIndices[j] + 2], 7) == 0b10000000) {

            if((ppu->scanLineSprites - ppu->oam[oamIndices[j]]) <= 7){
              bitPlane1 = readPpuBus(ppu, (spritePatternTableOffset + (((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) + 1) << 4) + (7 - (ppu->scanLineSprites - ppu->oam[oamIndices[j]]))));
              bitPlane2 = readPpuBus(ppu, (spritePatternTableOffset + (((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) + 1) << 4) + (7 - ((ppu->scanLineSprites - ppu->oam[oamIndices[j]]) + 8))));      
            } else if((ppu->scanLineSprites - ppu->oam[oamIndices[j]]) > 7){
              bitPlane1 = readPpuBus(ppu, (spritePatternTableOffset + ((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) << 4) + (7 - ((ppu->scanLineSprites - ppu->oam[oamIndices[j]]) - 8))));
              bitPlane2 = readPpuBus(ppu, (spritePatternTableOffset + ((((uint16_t) ppu->oam[oamIndices[j] + 1]) & 0xfe) << 4) + (7 - (ppu->scanLineSprites - ppu->oam[oamIndices[j]]))));    
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

        // if the colour isn't a transparency pixel, draw the pixel
        if(bitsCombined != 0){

          // if the background is transparent or the sprite is behind the backgrond, draw the pixel
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
          if(oamIndices[j] == 0 && bitsCombinedBackground != 0 && getBit(ppu->mask, 4) != 0 && getBit(ppu->status, 6) == 0 && i != 255){
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
  }

  // if background rendering is enabled, increment v
  if(getBit(ppu->mask, 3) != 0){
    // hori(v) = hori(t)
    ppu->vregister.vcomp.courseX = ppu->tregister.vcomp.courseX;
    ppu->vregister.vcomp.nameTableSelect = getBit(ppu->vregister.vcomp.nameTableSelect, 1) | getBit(ppu->tregister.vcomp.nameTableSelect, 0);

    incrementY(ppu);
  
    // gets shift registers ready for next scanline; fetches the first two tiles of the next line
    fetchFirstTwoTiles(ppu);
  }
 
}


void incrementY(PPU* ppu){
  if(ppu->vregister.vcomp.fineY < 7){
    ppu->vregister.vcomp.fineY++;
  } else {
    ppu->vregister.vcomp.fineY = 0;
    if(ppu->vregister.vcomp.courseY == 29){
      ppu->vregister.vcomp.courseY = 0;

      // toggle Y nametable select bit
      ppu->vregister.vcomp.nameTableSelect = (getBit(ppu->vregister.vcomp.nameTableSelect, 1) ^ 0b10) | getBit(ppu->vregister.vcomp.nameTableSelect, 0);
  
    } else if(ppu->vregister.vcomp.courseY == 31){
      ppu->vregister.vcomp.courseY = 0;

    } else {
      ppu->vregister.vcomp.courseY++;

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
void fillTempV(uint16_t *tempV, struct VComponent vcomp){
    *tempV = 0;
    *tempV = vcomp.courseX;
    *tempV = *tempV | (((uint16_t)vcomp.courseY) << 5);
    *tempV = *tempV | (((uint32_t) vcomp.nameTableSelect) << 10);
}



void incrementCourseX(PPU* ppu){
    if(ppu->vregister.vcomp.courseX == 31){
      ppu->vregister.vcomp.courseX = 0;
      ppu->vregister.vcomp.nameTableSelect = getBit(ppu->vregister.vcomp.nameTableSelect, 1) | (getBit(ppu->vregister.vcomp.nameTableSelect, 0) ^ 0b01);

    } else {
      ppu->vregister.vcomp.courseX++;
    }
  
}





void fetchFirstTwoTiles(PPU* ppu){

  
  uint16_t patternTableIndice;
  uint16_t tempV;
  uint8_t attributeTableByte;
  uint16_t patternTableOffset = 0;

  if(getBit(ppu->ctrl, 4) == 0){
    patternTableOffset = 0;
  } else if (getBit(ppu->ctrl, 4) != 0){
    patternTableOffset = 0x1000;
  }


  fillTempV(&tempV, ppu->vregister.vcomp);

  // fetch nametable
  patternTableIndice = readPpuBus(ppu, 0x2000 + tempV);


  // fetch attribute table byte 
  attributeTableByte = readPpuBus(ppu, 0x23c0 | (tempV & 0x0c00) | ((tempV >> 4) & 0x38) | ((tempV >> 2) & 0x07));
  attributeTableByte = findAndReturnAttributeByte(tempV, attributeTableByte);

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
  ppu->bitPlane1 = readPpuBus(ppu, (patternTableOffset + (patternTableIndice << 4) + ppu->vregister.vcomp.fineY));
  ppu->bitPlane1 = ppu->bitPlane1 << 8;

  ppu->bitPlane2 = readPpuBus(ppu, (patternTableOffset + (patternTableIndice << 4) + ppu->vregister.vcomp.fineY + 8));
  ppu->bitPlane2 = ppu->bitPlane2 << 8;

  incrementCourseX(ppu);



  fillTempV(&tempV, ppu->vregister.vcomp);
    // fetch pattern table ID
  patternTableIndice = readPpuBus(ppu, 0x2000 + tempV);

    // fetch attribute table byte
  attributeTableByte = readPpuBus(ppu, 0x23c0 | (tempV & 0x0c00) | ((tempV >> 4) & 0x38) | ((tempV >> 2) & 0x07));
  attributeTableByte = findAndReturnAttributeByte(tempV, attributeTableByte);
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
  ppu->bitPlane1 = ppu->bitPlane1 | readPpuBus(ppu, (patternTableOffset + (patternTableIndice << 4) + ppu->vregister.vcomp.fineY));
  ppu->bitPlane2 = ppu->bitPlane2 | readPpuBus(ppu, (patternTableOffset + (patternTableIndice << 4) + ppu->vregister.vcomp.fineY + 8));


   
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

  
  bus->ppu->vblank = 0;
  bus->ppu->frames++;


  // clears sprite 0 flag
  bus->ppu->status = clearBit(bus->ppu->status, 6);

  // clears sprite overflow flag
  bus->ppu->prerenderScanlineFlag = 1;


}



void prerenderScanline(Bus* bus){

  // if rendering is enabled
  if(getBit(bus->ppu->mask, 3) != 0){
    // hori(v) = hori(t)
    // copy all x components from t to v
    bus->ppu->vregister.vcomp.courseX = bus->ppu->tregister.vcomp.courseX;
    bus->ppu->vregister.vcomp.nameTableSelect = (bus->ppu->tregister.vcomp.nameTableSelect & 0b1);
  

    // vert(v) = vert(t)
    // copy all y components from t to v
    bus->ppu->vregister.vcomp.courseY = bus->ppu->tregister.vcomp.courseY;
    bus->ppu->vregister.vcomp.fineY = bus->ppu->tregister.vcomp.fineY;
    bus->ppu->vregister.vcomp.nameTableSelect = getBit(bus->ppu->vregister.vcomp.nameTableSelect, 0) | getBit(bus->ppu->tregister.vcomp.nameTableSelect, 1);

  
    fetchFirstTwoTiles(bus->ppu);
  }
  bus->ppu->status = clearBit(bus->ppu->status, 7);
  bus->ppu->status = clearBit(bus->ppu->status, 5);


  bus->ppu->prerenderScanlineFlag = 0;
  
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

uint8_t findAndReturnAttributeByte(uint16_t tempV, uint8_t attributeTableByte){

  int quadrant;

  uint8_t courseX = tempV & 0x1f;
  uint8_t courseY = ((tempV & 0x3e0) >> 5);

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