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


#include "memory.h"
#include "general.h"
#include "ppu.h"



void initMemStruct(Mem* mem, uint64_t size, enum DeviceType type, int inuse){
  if(inuse == TRUE){
    mem->contents = calloc(size, sizeof(uint8_t));
    mem->size = size;
  } else {
    mem->size = 0;
  }

  mem->type = type;
  mem->startAddr = 0;
  mem->endAddr = 0;
  mem->inuse = inuse;
  mem->mapped = 0;

  clearMem(mem);
}


void clearMem(Mem* mem){
  for(int i = 0; i < mem->size; ++i){
    mem->contents[i] = 0xff;
  }
}

// mapMemory()
// specifies where to map a block of memory on the bus.
// Must be initialized and apart and of the memArr array already, and is referenced
// with indexes.
//
// Memory blocks should be mapped in order of there place
// in the array when setting up a machine, or else an error will occur.
// NOTE: Only used in 6502 mode. Not needed for NES Mode. the memory mapping is fixed in NES mode.

void mapMemory(Bus* bus, uint16_t index, uint16_t addr){
  int mapMemoryFlag;

  Mem* mem = &(bus->memArr[index]);
  
  // TODO: implement error checking; most namely trying to map a block with not enough
  // space in the memory map

  if((((uint64_t)addr) + ((uint64_t)mem->size)) > 0xffff){
    puts("Memory out of bounds");
    return;
  }


  if(bus->numOfBlocks == 1 || index == 0){
    bus->memArr[0].startAddr = addr;
    bus->memArr[0].endAddr = addr + bus->memArr[0].size;
    bus->memArr[0].mapped = 1;
    printf("Bank %d mapped to address %x \n", index, mem->startAddr);
    printf("Ending at address %x \n", mem->endAddr);
    return;
  } else if(bus->numOfBlocks == 0){
    return;
  } 


  if((mem - 1)->mapped == 0){
    puts("Error: Make sure to map memory banks in order");
    return;
  }

  // check to see if the address we are mapping too is out of bounds of existing mapped memory
  if(((mem - 1)->endAddr < addr)){
    mem->startAddr = addr;
    mem->endAddr = addr + mem->size;
    printf("Bank %d mapped to address %x \n", index, mem->startAddr);
    printf("Ending at address %x \n", mem->endAddr);
  }  
}


// inits the bus and the things connected to it
void initBus(Bus* bus, uint16_t banks){
  bus->memArr = calloc(banks, sizeof(Mem));
  bus->cpu = (CPU*) malloc(sizeof(CPU));
  bus->ppu = (PPU*) malloc(sizeof(PPU));
  bus->numOfBlocks = banks;
  bus->controller1.latchedButtons = 0x00;
  bus->controller1.strobed = 0;
  bus->controller1.readCount = 0;
  bus->controller1.sdlButtons = 0x00;

  bus->controller2.latchedButtons = 0x00;
  bus->controller2.strobed = 0;
  bus->controller2.readCount = 0;
  bus->controller2.sdlButtons = 0x00;



  bus->bankSelect = 0;
}

#if NESEMU == 0
void writeBus(Bus* bus, uint16_t addr, uint8_t val){
  if(bus->numOfBlocks == 0){
    return;
  }
  /*
  if(addr >= 0x0000 && addr <= bus->memArr[0].size){
    if(bus->memArr[0].type == Ram){
      bus->memArr[0].contents[addr] = val;
    }
  } else {
    if(bus->memArr[1].type == Ram){
      bus->memArr[1].contents[addr + bus->memArr[0].size - 1] = val;
    }
  }

  */
  uint16_t index;
  for(int i = 0; i < bus->numOfBlocks; ++i){
    if(bus->memArr[i].startAddr <= addr && bus->memArr[i].endAddr >= addr){

      if(bus->memArr[i].type == Rom)
        break;

      bus->memArr[i].contents[addr] = val;
      //printf("Writing to %d with %d \n", addr, val);
      break;
    } 
  }



}

#elif NESEMU == 1

// hard-coded for nes memory map
// does not make use of the bounds checking of the starAddr and endAddr for each Mem struct
void writeBus(Bus* bus, uint16_t addr, uint8_t val){
  int prevVal;
  
  if(addr >= 0 && addr <= 0x07ff){
    bus->memArr[0].contents[addr] = val;
  } else if(addr >= 0x0800 && addr <= 0x0fff){
    bus->memArr[0].contents[addr - 0x800] = val;
  } else if(addr >= 0x1000 && addr <= 0x17ff){
    bus->memArr[0].contents[addr - 0x1000] = val;
  } else if(addr >= 0x1800 && addr <= 0x1fff){
    bus->memArr[0].contents[addr - 0x1800] = val;
  } else if(addr >= 0x2000 && addr <= 0x3fff){
    switch(((addr % 8) + 0x2000)){
      case 0x2000:
        bus->ppu->ctrl = val & 0xfc;
        bus->ppu->tregister.vcomp.nameTableSelect = (val & 0b11);
        break;
      case 0x2001:
        bus->ppu->mask = val;
        break;
      case 0x2002:
        return;
      case 0x2003:
        bus->ppu->oamaddr = val;
        break;
      case 0x2004:
        bus->ppu->oam[bus->ppu->oamaddr] = val;
        break;
      case 0x2005:
        if(bus->ppu->wregister == 0){
          bus->ppu->xregister = (val & 0x07);
          bus->ppu->tregister.vcomp.courseX = ((val & 0xf8) >> 3);
          bus->ppu->wregister = 1;
        } else if(bus->ppu->wregister == 1){
          bus->ppu->tregister.vcomp.courseY = ((val & 0xf8) >> 3);
          bus->ppu->tregister.vcomp.fineY = (val & 0x07);
          bus->ppu->wregister = 0;
        }
        break;
      case 0x2006:

         if(bus->ppu->wregister == 0){

          bus->ppu->tregister.vreg = (((uint16_t)val & 0x3f) << 8);
          bus->ppu->tregister.vreg = clearBit16bit(bus->ppu->tregister.vreg, 14);
          bus->ppu->wregister = 1;
        } else if(bus->ppu->wregister == 1){
          bus->ppu->tregister.vreg = bus->ppu->tregister.vreg | ((uint16_t) val);
          bus->ppu->vregister.vreg = bus->ppu->tregister.vreg;
          bus->ppu->wregister = 0;
        }
        
        break;
      case 0x2007:
        
        writePpuBus(bus->ppu, bus->ppu->vregister.vreg, val);
        if(getBit(bus->ppu->ctrl, 2) == 0){
          bus->ppu->vregister.vreg++;
          
        } else if(getBit(bus->ppu->ctrl, 2) != 0){
          bus->ppu->vregister.vreg += 32;
        }

        break;
    }
  } else if(addr == 0x4014){
    bus->ppu->oamdma = val;
    dmaTransfer(bus);
  } else if(addr == 0x4016){

    if(bus->controller1.strobed == 0 && val == 1){
      bus->controller1.strobed = 1;
    } else if(bus->controller1.strobed == 1 && val == 0){
      bus->controller1.strobed = 0;
      bus->controller1.latchedButtons = bus->controller1.sdlButtons;
      bus->controller1.readCount = 0;
    } 

    return;
    
  } else if(addr == 0x4017){
    if(bus->controller2.strobed == 0 && val == 1){
      bus->controller2.strobed = 1;
    } else if(bus->controller2.strobed == 1 && val == 0){
      bus->controller2.strobed = 0;
      bus->controller2.latchedButtons = bus->controller2.sdlButtons;
      bus->controller2.readCount = 0;
    } 

  } else {
    
    // NROM mapper (the basic bitch mapper)
    switch(bus->mapper){
      case 0:
        if(addr >= 0x8000 && addr <= 0xffff){
          return;
        }
        break;
      case 2:
        if(addr >= 0x8000 && addr <= 0xffff){
          bus->bankSelect = val;
          bus->bankSelect = bus->bankSelect & 0xf;
        }
      }
    }
  

};

#endif


#if NESEMU == 0

uint8_t readBus(Bus* bus, uint16_t addr){
  if(bus->numOfBlocks == 0){
    return 0;
  }

  for(int i = 0; i < bus->numOfBlocks; ++i){
    if(bus->memArr[i].startAddr <= addr && bus->memArr[i].endAddr >= addr){
      //printf("Bus memory block %d being read \n", i);
      return bus->memArr[i].contents[addr - bus->memArr[i].startAddr];
    } 
  }
  return 0;
/*  if(addr >= 0x0000 && addr <= bus->memArr[0].size){
    return bus->memArr[0].contents[addr];
  } else {
    return bus->memArr[1].contents[addr + bus->memArr[0].size - 1];
  }
  */

}

#elif NESEMU == 1

// hard-coded for nes memory map
// does not make use of the bounds checking of the starAddr and endAddr for each Mem struct
uint8_t readBus(Bus* bus, uint16_t addr){
  //printf("Reading address %x \n", addr);
  uint8_t temp;
  uint16_t temp16;
  if(bus->numOfBlocks == 0){
    return 0;

  }
  if(addr >= 0 && addr <= 0x07ff){
    return bus->memArr[0].contents[addr];
  } else if(addr >= 0x0800 && addr <= 0x0fff){
    return bus->memArr[0].contents[addr - 0x800];
  } else if(addr >= 0x1000 && addr <= 0x17ff){
    return bus->memArr[0].contents[addr - 0x1000];
  } else if(addr >= 0x1800 && addr <= 0x1fff){
    return bus->memArr[0].contents[addr - 0x1800];
  } else if(addr >= 0x2000 && addr <= 0x3fff){
    switch(((addr % 8) + 0x2000)){
      case 0x2000:
        return 0;
      case 0x2001:
        return 0;
      case 0x2002:
        temp = bus->ppu->status;
        //printf("Reading status \n");
        //bus->ppu->status = clearBit(bus->ppu->status, 7);
        bus->ppu->wregister = 0;
        return temp;
      case 0x2003:
        return 0;
      case 0x2004:
        //printf("Reading oamdata \n");
        return bus->ppu->oam[bus->ppu->oamaddr];
      case 0x2005:
        return 0;
      case 0x2006:
        return 0;
      case 0x2007:
        temp = bus->ppu->data;
        bus->ppu->data = readPpuBus(bus->ppu, bus->ppu->vregister.vreg);
        if(getBit(bus->ppu->ctrl, 2) == 0){
          bus->ppu->vregister.vreg++;
          
        } else if(getBit(bus->ppu->ctrl, 2) != 0){
          bus->ppu->vregister.vreg += 32;
        }
        return temp;
    }
  } else if(addr == 0x4016){
    if(bus->controller1.strobed == 0){
      switch(bus->controller1.readCount){
        case 0:
          bus->controller1.readCount++;
          return getBit(bus->controller1.latchedButtons, 0);
        case 1:
          bus->controller1.readCount++;
          return getBit(bus->controller1.latchedButtons, 1) >> 1;
        case 2:
          bus->controller1.readCount++;
          return getBit(bus->controller1.latchedButtons, 2) >> 2;
        case 3:
          bus->controller1.readCount++;
          return getBit(bus->controller1.latchedButtons, 3) >> 3;
        case 4:
          bus->controller1.readCount++;
          return getBit(bus->controller1.latchedButtons, 4) >> 4;
        case 5:
          bus->controller1.readCount++; 
          return getBit(bus->controller1.latchedButtons, 5) >> 5;
        case 6:
          bus->controller1.readCount++;
          return getBit(bus->controller1.latchedButtons, 6) >> 6;
        case 7:
          bus->controller1.readCount = 0;
          return getBit(bus->controller1.latchedButtons, 7) >> 7;
        default:
          return 0;
      }
    } else {
      return 0;
    }
  } else if(addr == 0x4017){
    if(bus->controller2.strobed == 0){
      switch(bus->controller2.readCount){
        case 0:
          bus->controller2.readCount++;
          temp = getBit(bus->controller2.latchedButtons, 0);
          break;
        case 1:
          bus->controller1.readCount++;
          temp = getBit(bus->controller2.latchedButtons, 1) >> 1;
          break;
        case 2:
          bus->controller1.readCount++;
          temp = getBit(bus->controller2.latchedButtons, 2) >> 2;
          break;
        case 3:
          bus->controller1.readCount++;
          temp = getBit(bus->controller2.latchedButtons, 3) >> 3;
          break;
        case 4:
          bus->controller1.readCount++;
          temp = getBit(bus->controller2.latchedButtons, 4) >> 4;
          break;
        case 5:
          bus->controller1.readCount++; 
          temp = getBit(bus->controller2.latchedButtons, 5) >> 5;
          break;
        case 6:
          bus->controller1.readCount++;
          temp = getBit(bus->controller2.latchedButtons, 6) >> 6;
          break;
        case 7:
          bus->controller1.readCount = 0;
          temp = getBit(bus->controller2.latchedButtons, 7) >> 7;
          break;
        default:
          return 0;
      }
      if(bus->controller2.lightSensor == 0){
         temp = clearBit(temp, 3);
      } else if(bus->controller2.lightSensor == 1) {
         temp = setBit(temp, 3);
      }
      if(bus->controller2.triggerPulled == 0){
         temp = clearBit(temp, 4);
      } else if(bus->controller2.triggerPulled == 1){
         temp = setBit(temp, 4);
      }
      return temp;

    } else {
      return 0;
    }
  } else {

    switch(bus->mapper){
      // NROM mapper
      case 0:
        if(addr >= 0x8000 && addr <= 0xffff){
          return bus->memArr[1].contents[addr - 0x8000];
        }
        break;
      case 2:
        if(addr >= 0x8000 && addr <= 0xbfff){
          return bus->memArr[bus->bankSelect + 1].contents[addr - 0x8000];
        } else if(addr >= 0xc000 && addr <= 0xffff){
          return bus->memArr[bus->numOfBlocks - 1].contents[addr - 0xc000];
        }
        
      
    }
  }

}

#endif

// ***** PPU Memory operations ******

void initPpuBus(PPUBus* bus, int banks){
  bus->memArr = calloc(banks, sizeof(Mem));
  bus->numOfBlocks = banks;
}
#if NESEMU == 0

uint8_t readPpuBus(PPU* ppu, uint16_t addr){
  if(ppu->ppubus->numOfBlocks == 0){
    return 0;
  }
  for(int i = 0; i < ppu->ppubus->numOfBlocks; ++i){
    if(ppu->ppubus->memArr[i].startAddr <= addr && ppu->ppubus->memArr[i].endAddr >= addr){
      //printf("Bus memory block %d being read \n", i);
      return ppu->ppubus->memArr[i].contents[addr - ppu->ppubus->memArr[i].startAddr];
    } 

  }
  return 0;


}



void writePpuBus(PPU* ppu, uint16_t addr, uint8_t val){
  Mem* mem;
  if(ppu->ppubus->numOfBlocks == 0){
    return;
  }

  for(int i = 0; i < ppu->ppubus->numOfBlocks; ++i){
    mem = &(ppu->ppubus->memArr[i]);
    if(mem->startAddr <= addr && mem->endAddr >= addr){

      if(mem->type == CHRRom)
        break;

      mem->contents[addr] = val;
      //printf("Writing to %d with %d \n", addr, val);
      break;
    } 
  }


}
#elif NESEMU == 1

uint8_t readPpuBus(PPU* ppu, uint16_t addr){
  if(addr >= 0x0000 && addr <= 0x1fff){
    return ppu->chrrom[addr];
  } else if(addr >= 0x2000 && addr <= 0x2fff){
    return ppu->vram[addr - 0x2000];
  } else if (addr >= 0x3000 && addr <= 0x3eff){
    return ppu->vram[addr - 0x3000];
  } else if (addr >= 0x3f00 && addr <= 0x3f1f){
    return ppu->paletteram[addr - 0x3f00];
  } else if(addr >= 0x3f20 && addr <= 0x3f3f){
    return ppu->paletteram[addr - 0x3f20];
  } else if(addr >= 0x3f40 && addr <= 0x3f5f){
    return ppu->paletteram[addr - 0x3f40];
  } else if(addr >= 0x3f60 && addr <= 0x3f7f){
    return ppu->paletteram[addr - 0x3f60];
  } else if(addr >= 0x3f80 && addr <= 0x3f9f){
    return ppu->paletteram[addr - 0x3f80];
  } else if(addr >= 0x3fa0 && addr <= 0x3fBf){
    return ppu->paletteram[addr - 0x3fa0];
  } else if(addr >= 0x3fc0 && addr <= 0x3fdf){
    return ppu->paletteram[addr - 0x3fc0];
  } else if(addr >= 0x3fe0 && addr <= 0x3fff){
    return ppu->paletteram[addr - 0x3fe0];
  } 

}


void writePpuBus(PPU* ppu, uint16_t addr, uint8_t val){
  //printf("Writing to PPU address %x with value %x \n", addr, val);
  if(addr >= 0x0000 && addr <= 0x1fff){
    if(ppu->flagChrRam == 1){
      ppu->chrrom[addr] = val;
    } else {
      return;
    }
    
  } else if(addr >= 0x2000 && addr <= 0x2fff){


    ppu->vram[addr - 0x2000] = val;

    // vertical arrangement (horizontal mirroring)
    if(ppu->mirroring == 0){
      if(addr >= 0x2000 && addr <= 0x23bf){
        ppu->vram[(addr - 0x2000) + 0x400] = val;
      } else if(addr >= 0x2400 && addr <= 0x27bf){
        ppu->vram[(addr - 0x2000) - 0x400] = val;
      } else if(addr >= 0x2800 && addr <= 0x2bbf){
        ppu->vram[(addr - 0x2000) + 0x400] = val;
      } else if(addr >= 0x2c00 && addr <= 0x2fbf){
        ppu->vram[(addr - 0x2000) - 0x400] = val;
      }
      // horizontal arrangement (vertical mirroring)
    } else if(ppu->mirroring == 1){

      if(addr >= 0x2000 && addr <= 0x23bf){
        ppu->vram[(addr - 0x2000) + 0x800] = val;
      } else if(addr >= 0x2400 && addr <= 0x27bf){
        ppu->vram[(addr - 0x2000) + 0x800] = val;
      } else if(addr >= 0x2800 && addr <= 0x2bbf){
        ppu->vram[(addr - 0x2000) - 0x800] = val;
      } else if(addr >= 0x2c00 && addr <= 0x2fbf){
        ppu->vram[(addr - 0x2000) - 0x800] = val;
      }
    
    }

  } else if (addr >= 0x3000 && addr <= 0x3eff){
    ppu->vram[addr - 0x3000] = val;
  } else if (addr >= 0x3f00 && addr <= 0x3f1f){

    // palette mirroring
    switch(addr){
      case 0x3f00:
        ppu->paletteram[(addr - 0x3f00) + 0x10] = val;
        break;
      case 0x3f10:
        ppu->paletteram[(addr - 0x3f00) - 0x10] = val;
        break;
      case 0x3f04:
        ppu->paletteram[(addr - 0x3f00) + 0x10] = val;
        break;
      case 0x3f14:
        ppu->paletteram[(addr - 0x3f00) - 0x10] = val;
        break;
      case 0x3f08:
        ppu->paletteram[(addr - 0x3f00) + 0x10] = val;
        break;
      case 0x3f18:
        ppu->paletteram[(addr - 0x3f00) - 0x10] = val;
        break;
      case 0x3f0c:
        ppu->paletteram[(addr - 0x3f00) + 0x10] = val;
        break;
      case 0x3f1c:
        ppu->paletteram[(addr - 0x3f00) - 0x10] = val;
        break;
      default:
        break;
    }
  
    ppu->paletteram[addr - 0x3f00] = val;
  } else if(addr >= 0x3f20 && addr <= 0x3f3f){
    switch(addr){
      case 0x3f20:
        ppu->paletteram[(addr - 0x3f20) + 0x10] = val;
        break;
      case 0x3f30:
        ppu->paletteram[(addr - 0x3f20) - 0x10] = val;
        break;
      case 0x3f24:
        ppu->paletteram[(addr - 0x3f20) + 0x10] = val;
        break;
      case 0x3f34:
        ppu->paletteram[(addr - 0x3f20) - 0x10] = val;
        break;
      case 0x3f28:
        ppu->paletteram[(addr - 0x3f20) + 0x10] = val;
        break;
      case 0x3f38:
        ppu->paletteram[(addr - 0x3f20) - 0x10] = val;
        break;
      case 0x3f2c:
        ppu->paletteram[(addr - 0x3f20) + 0x10] = val;
        break;
      case 0x3f3c:
        ppu->paletteram[(addr - 0x3f20) - 0x10] = val;
        break;
      default:
        break;
    }
    ppu->paletteram[addr - 0x3f20] = val;
  } else if(addr >= 0x3f40 && addr <= 0x3f5f){
    switch(addr){
      case 0x3f40:
        ppu->paletteram[(addr - 0x3f40) + 0x10] = val;
        break;
      case 0x3f50:
        ppu->paletteram[(addr - 0x3f40) - 0x10] = val;
        break;
      case 0x3f44:
        ppu->paletteram[(addr - 0x3f40) + 0x10] = val;
        break;
      case 0x3f54:
        ppu->paletteram[(addr - 0x3f40) - 0x10] = val;
        break;
      case 0x3f48:
        ppu->paletteram[(addr - 0x3f40) + 0x10] = val;
        break;
      case 0x3f58:
        ppu->paletteram[(addr - 0x3f40) - 0x10] = val;
        break;
      case 0x3f4c:
        ppu->paletteram[(addr - 0x3f40) + 0x10] = val;
        break;
      case 0x3f5c:
        ppu->paletteram[(addr - 0x3f40) - 0x10] = val;
        break;
      default:
        break;
    }
    ppu->paletteram[addr - 0x3f40] = val;
  } else if(addr >= 0x3f60 && addr <= 0x3f7f){
    switch(addr){
      case 0x3f60:
        ppu->paletteram[(addr - 0x3f60) + 0x10] = val;
        break;
      case 0x3f70:
        ppu->paletteram[(addr - 0x3f60) - 0x10] = val;
        break;
      case 0x3f64:
        ppu->paletteram[(addr - 0x3f60) + 0x10] = val;
        break;
      case 0x3f74:
        ppu->paletteram[(addr - 0x3f60) - 0x10] = val;
        break;
      case 0x3f68:
        ppu->paletteram[(addr - 0x3f60) + 0x10] = val;
        break;
      case 0x3f78:
        ppu->paletteram[(addr - 0x3f60) - 0x10] = val;
        break;
      case 0x3f6c:
        ppu->paletteram[(addr - 0x3f60) + 0x10] = val;
        break;
      case 0x3f7c:
        ppu->paletteram[(addr - 0x3f60) - 0x10] = val;
        break;
      default:
        break;
    }
    ppu->paletteram[addr - 0x3f60] = val;
  } else if(addr >= 0x3f80 && addr <= 0x3f9f){
    switch(addr){
      case 0x3f80:
        ppu->paletteram[(addr - 0x3f80) + 0x10] = val;
        break;
      case 0x3f90:
        ppu->paletteram[(addr - 0x3f80) - 0x10] = val;
        break;
      case 0x3f84:
        ppu->paletteram[(addr - 0x3f80) + 0x10] = val;
        break;
      case 0x3f94:
        ppu->paletteram[(addr - 0x3f80) - 0x10] = val;
        break;
      case 0x3f88:
        ppu->paletteram[(addr - 0x3f80) + 0x10] = val;
        break;
      case 0x3f98:
        ppu->paletteram[(addr - 0x3f80) - 0x10] = val;
        break;
      case 0x3f8c:
        ppu->paletteram[(addr - 0x3f80) + 0x10] = val;
        break;
      case 0x3f9c:
        ppu->paletteram[(addr - 0x3f80) - 0x10] = val;
        break;
      default:
        break;
    }
    ppu->paletteram[addr - 0x3f80] = val;
  } else if(addr >= 0x3fa0 && addr <= 0x3fbf){
    switch(addr){
      case 0x3fa0:
        ppu->paletteram[(addr - 0x3fa0) + 0x10] = val;
        break;
      case 0x3fb0:
        ppu->paletteram[(addr - 0x3fa0) - 0x10] = val;
        break;
      case 0x3fa4:
        ppu->paletteram[(addr - 0x3fa0) + 0x10] = val;
        break;
      case 0x3fb4:
        ppu->paletteram[(addr - 0x3fa0) - 0x10] = val;
        break;
      case 0x3fa8:
        ppu->paletteram[(addr - 0x3fa0) + 0x10] = val;
        break;
      case 0x3fb8:
        ppu->paletteram[(addr - 0x3fa0) - 0x10] = val;
        break;
      case 0x3fac:
        ppu->paletteram[(addr - 0x3fa0) + 0x10] = val;
        break;
      case 0x3fbc:
        ppu->paletteram[(addr - 0x3fa0) - 0x10] = val;
        break;
      default:
        break;
    }
    ppu->paletteram[addr - 0x3fa0] = val;
  } else if(addr >= 0x3fc0 && addr <= 0x3fdf){
    switch(addr){
      case 0x3fc0:
        ppu->paletteram[(addr - 0x3fc0) + 0x10] = val;
        break;
      case 0x3fd0:
        ppu->paletteram[(addr - 0x3fc0) - 0x10] = val;
        break;
      case 0x3fc4:
        ppu->paletteram[(addr - 0x3fc0) + 0x10] = val;
        break;
      case 0x3fd4:
        ppu->paletteram[(addr - 0x3fc0) - 0x10] = val;
        break;
      case 0x3fc8:
        ppu->paletteram[(addr - 0x3fc0) + 0x10] = val;
        break;
      case 0x3fd8:
        ppu->paletteram[(addr - 0x3fc0) - 0x10] = val;
        break;
      case 0x3fcc:
        ppu->paletteram[(addr - 0x3fc0) + 0x10] = val;
        break;
      case 0x3fdc:
        ppu->paletteram[(addr - 0x3fc0) - 0x10] = val;
        break;
      default:
        break;
    }
    ppu->paletteram[addr - 0x3fc0] = val;
  } else if(addr >= 0x3fe0 && addr <= 0x3fff){
    switch(addr){
      case 0x3fe0:
        ppu->paletteram[(addr - 0x3fe0) + 0x10] = val;
        break;
      case 0x3ff0:
        ppu->paletteram[(addr - 0x3fe0) - 0x10] = val;
        break;
      case 0x3fe4:
        ppu->paletteram[(addr - 0x3fe0) + 0x10] = val;
        break;
      case 0x3ff4:
        ppu->paletteram[(addr - 0x3fe0) - 0x10] = val;
        break;
      case 0x3fe8:
        ppu->paletteram[(addr - 0x3fe0) + 0x10] = val;
        break;
      case 0x3ff8:
        ppu->paletteram[(addr - 0x3fe0) - 0x10] = val;
        break;
      case 0x3fec:
        ppu->paletteram[(addr - 0x3fe0) + 0x10] = val;
        break;
      case 0x3ffc:
        ppu->paletteram[(addr - 0x3fe0) - 0x10] = val;
        break;
      default:
        break;
    }
    ppu->paletteram[addr - 0x3fe0] = val;
  } 


}

#endif

void mapPpuMemory(PPUBus* bus, uint16_t index, uint16_t addr){
  int mapMemoryFlag;
  Mem* mem = &(bus->memArr[index]);
  
  // TODO: implement error checking; most namely trying to map a block with not enough
  // space in the memory map

  if((((uint64_t)addr) + ((uint64_t)mem->size)) > 0xffff){
    puts("Memory out of bounds");
    return;
  }


  if(bus->numOfBlocks == 1 || index == 0){
    bus->memArr[0].startAddr = addr;
    bus->memArr[0].endAddr = addr + bus->memArr[0].size;
    bus->memArr[0].mapped = 1;
    printf("Bank %d mapped to address %x \n", index, mem->startAddr);
    printf("Ending at address %x \n", mem->endAddr);
    return;
  } else if(bus->numOfBlocks == 0){
    return;
  } 


  if((mem - 1)->mapped == 0){
    puts("Error: Make sure to map memory banks in order");
    return;
  }

  // check to see if the address we are mapping too is out of bounds of existing mapped memory
  if(((mem - 1)->endAddr < addr)){
    mem->startAddr = addr;
    mem->endAddr = addr + mem->size;
    printf("Bank %d mapped to address %x \n", index, mem->startAddr);
    printf("Ending at address %x \n", mem->endAddr);
  }  
}
