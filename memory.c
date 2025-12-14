#include "memory.h"
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
  bus->controller2.sdlButtons = 0x00;

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
        bus->ppu->ctrl = val;
        break;
      case 0x2001:
        bus->ppu->mask = val;
        break;
      case 0x2002:
        bus->ppu->status = val;
        break;
      case 0x2003:
        bus->ppu->oamaddr = val;
        break;
      case 0x2004:
        bus->ppu->oam[bus->ppu->oamaddr] = val;
        break;
      case 0x2005:
        if(bus->ppu->wregister == 1){
          bus->ppu->scroll = bus->ppu->scroll | (uint16_t) val;
          bus->ppu->wregister = 0;
        } else if(bus->ppu->wregister == 0){
          bus->ppu->scroll = bus->ppu->scroll | (((uint16_t)val) << 8);
          bus->ppu->wregister = 1;
        }
        break;
      case 0x2006:
        //printf("Writing to bus at $2006 with %x \n", val);

         if(bus->ppu->wregister == 0){
          bus->ppu->vregister1 = (((uint16_t)val) << 8);
          bus->ppu->wregister = 1;
        } else if(bus->ppu->wregister == 1){
          bus->ppu->vregister1 = bus->ppu->vregister1 | (uint16_t)val;
          bus->ppu->wregister = 0;
        }
        
        break;
      case 0x2007:
        
        bus->ppu->data = val;
        int overflowFlag;
        //printf("Writing to bus at $2007 \n");
        writePpuBus(bus->ppu, bus->ppu->vregister1, bus->ppu->data);
        //printf("Wrote to ppu bus %x with value %x \n", bus->ppu->vregister1, bus->ppu->data);
        if(getBit(bus->ppu->ctrl, 2) == 0){
          bus->ppu->vregister1++;
          
        } else if(getBit(bus->ppu->ctrl, 2) != 0){
          bus->ppu->vregister1 += 32;
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
    
  }  else {

    // NROM mapper (the basic bitch mapper)
    if(bus->mapper == 0){
      if(addr >= 0x8000 && addr <= 0xffff){
        bus->memArr[1].contents[addr - 0x8000] = val;
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
        return bus->ppu->ctrl;
      case 0x2001:
        return bus->ppu->mask;
      case 0x2002:
        //printf("Reading status \n");
        temp = bus->ppu->status;
        bus->ppu->status = clearBit(bus->ppu->status, 7);
        return temp;
      case 0x2003:
        return bus->ppu->oamaddr;
      case 0x2004:
        //printf("Reading oamdata \n");
        return bus->ppu->oam[bus->ppu->oamaddr];
      case 0x2005:
        if(bus->ppu->wregister == 0){
          return bus->ppu->scroll & 0xff;
        } else if(bus->ppu->wregister == 1){
          return ((bus->ppu->scroll & 0xff00) >> 8);
        }
      case 0x2006:
         if(bus->ppu->wregister == 0){
          return bus->ppu->addr & 0xff;
        } else if(bus->ppu->wregister == 1){
          return ((bus->ppu->addr & 0xff00) >> 8);
        }
      case 0x2007:
        return bus->ppu->data;
    }
  } else if(addr == 0x4016){
    //printf("reading controller one \n");
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
    //printf("reading controller two \n");
    return 0;
  } else {

    // NROM mapper 
    if(bus->mapper == 0){
      if(addr >= 0x8000 && addr <= 0xffff){
        return bus->memArr[1].contents[addr - 0x8000];
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
  } 

}


void writePpuBus(PPU* ppu, uint16_t addr, uint8_t val){
  //printf("Writing to PPU address %x \n", addr);
  if(addr >= 0x0000 && addr <= 0x1fff){
    ppu->chrrom[addr] = val;
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
    ppu->paletteram[addr - 0x3f00] = val;
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
