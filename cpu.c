#include "cpu.h"


int pageFlag;


// to start a cpu from an outsider, this function is to be used.
//
// int cond is used to pass information on how the cpu should run 
//
// the first bit of int cond is a flag for step through or run as fast as possible
int execute(CPU* cpu, Bus* bus, int cond){

  
  return 0;


}

void reset(CPU* cpu, Bus* bus){
    cpu->a = 0;
    cpu->x = 0;
    cpu->y = 0;
    cpu->sp = 0;
    cpu->pf = 0;


    //reads reset vector to find starting pc
    
   //reads lower byte (little endian)
    cpu->pc = (uint16_t)readBus(bus, 0xfffc);

    //reads upper byte (little endian)
    cpu->pc = cpu->pc | ((uint16_t)readBus(bus, 0xfffd) << 8);
    
    bus->controller1.strobe = 0;
    bus->controller1.strobeCount = 0;
    bus->controller1.buttons = 0x00;
    bus->controller2.buttons = 0x00;
    cpu->nmiInterruptFlag = 0;
    
    cpu->cycles = 0;
    //printf("cpu->pc %x \n", cpu->pc);
    

}

void triggerNmi(CPU* cpu){
  cpu->nmiInterruptFlag = 1;
}

void checkForInterrupts(CPU* cpu, Bus* bus){
  if(cpu->nmiInterruptFlag == 1){
    nmi(cpu, bus);
    cpu->nmiInterruptFlag = 0;
  } 
}


// Differences between nmi and irq interrupts:
//   irq will only be honored if the interrupt disable flag is not set, 
//   whereas the nmi will ignore this flag and will always do an interupt.
//
//
//
int nmi(CPU* cpu, Bus* bus){
  //printf("NMI triggered! \n");

  uint16_t temp;
  //printf("cpu->pc at nmi %x \n", cpu->pc);

  // push the msb and lsb of the program counter onto the stack
  pushStack(cpu, bus, (uint8_t)((cpu->pc & 0xff00) >> 8));

  pushStack(cpu, bus, (uint8_t)(cpu->pc & 0x00ff));

  // pushes the processor flags onto the stack
  //printf("CPU->pf %x \n", cpu->pf);
  pushStack(cpu, bus, cpu->pf);

  // sets the interupt disable flag
  setBit(cpu->pf, 2);


  cpu->pc = readBus(bus, 0xfffa);
  temp = (uint16_t)readBus(bus, 0xfffb);
  temp = temp << 8;
  cpu->pc = cpu->pc | temp;

  return 7;





}


int irq(CPU* cpu, Bus* bus){

  if(((getBit(cpu->pf, 2) >> 2) == 1)){
    return 0; 
  };

  // break flag set to be prepared when pushed onto the stack
  cpu->pf = setBit(cpu->pf, 4);

 
  uint16_t temp;

  // push the msb and lsb of the program counter onto the stack
  pushStack(cpu, bus, (uint8_t)(cpu->pc & 0x00ff));
  pushStack(cpu, bus, (uint8_t)(cpu->pc & 0xff00));

  // pushes the processor flags onto the stack
  pushStack(cpu, bus, cpu->pf);

  // sets the interupt disable flag
  cpu->pf = setBit(cpu->pf, 2);


  // break flag is now cleared because it only exists within the stack
  cpu->pf = clearBit(cpu->pf, 4);


  cpu->pc = readBus(bus, 0xfffe);
  temp = (uint16_t)readBus(bus, 0xffff);
  temp = temp << 8;
  cpu->pc = cpu->pc | temp;
  //printf("Setting pc to %d \n", cpu->pc);


  return 7;
 


}


// returns how many cycles have been executed
int decodeAndExecute(CPU* cpu, Bus* bus, uint8_t oppCode){
  int cyclesCompleted;
  //printf("\t Executing oppcode: %x at %x \n", oppCode, cpu->pc);
  switch(oppCode){
    case 0x00:
      cyclesCompleted = brki(cpu, bus);
      break;
    case 0x01:
      cyclesCompleted = ora(cpu, bus, indirectX); 
      break;
    case 0x05:
      cyclesCompleted = ora(cpu, bus, zeroPage);
      break;
    case 0x06:
      cyclesCompleted = asl(cpu, bus, zeroPage);
      break;
    case 0x08:
      cyclesCompleted = php(cpu, bus);
      break;
    case 0x09:
      cyclesCompleted = ora(cpu, bus, immediate);
      break;
    case 0x0a:
      cyclesCompleted = asl(cpu, bus, accumulator);
      break;
    case 0x0d:
      cyclesCompleted = ora(cpu, bus, absolute);
      break;
    case 0x0e:
      cyclesCompleted = asl(cpu, bus, absolute);
      break;
    case 0x10:
      cyclesCompleted = bpl(cpu, bus);
      break;
    case 0x11:
      cyclesCompleted = ora(cpu, bus, indirectY);
      break;
    case 0x15:
      cyclesCompleted = ora(cpu, bus, zeroPageX);
      break;
    case 0x16:
      cyclesCompleted = asl(cpu, bus, zeroPageX);
      break;
    case 0x18:
      cyclesCompleted = clc(cpu);
      break;
    case 0x19:
      cyclesCompleted = ora(cpu, bus, absoluteY);
      break;
    case 0x1d:
      cyclesCompleted = ora(cpu, bus, absoluteX);
      break;
    case 0x1e:
      cyclesCompleted = asl(cpu, bus, absoluteX);
      break;
    case 0x20:
      cyclesCompleted = jsr(cpu, bus);
      break;
    case 0x21:
      cyclesCompleted = and(cpu, bus, indirectX);
      break;
    case 0x24:
      cyclesCompleted = bit(cpu, bus, zeroPage);
      break;
    case 0x25:
      cyclesCompleted = and(cpu, bus, zeroPage);
      break;
    case 0x26:
      cyclesCompleted = rol(cpu, bus, zeroPage);
      break;
    case 0x28:
      cyclesCompleted = plp(cpu, bus);
      break;
    case 0x29:
      cyclesCompleted = and(cpu, bus, immediate);
      break;
    case 0x2a:
      cyclesCompleted = rol(cpu, bus, accumulator);
      break;
    case 0x2c:
      cyclesCompleted = bit(cpu, bus, absolute);
      break;
    case 0x2d:
      cyclesCompleted = and(cpu, bus, absolute);
      break;
    case 0x2e:
      cyclesCompleted = rol(cpu, bus, absolute);
      break;
    case 0x30:
      cyclesCompleted = bmi(cpu, bus);
      break;
    case 0x31:
      cyclesCompleted = and(cpu, bus, indirectY);
      break;
    case 0x35:
      cyclesCompleted = and(cpu, bus, zeroPageX);
      break;
    case 0x36:
      cyclesCompleted = rol(cpu, bus, zeroPageX);
      break;
    case 0x38:
      cyclesCompleted = sec(cpu, bus);
      break;
    case 0x39:
      cyclesCompleted = and(cpu, bus, absoluteY);
      break;
    case 0x3d:
      cyclesCompleted = and(cpu, bus, absoluteX);
      break;
    case 0x3e:
      cyclesCompleted = rol(cpu, bus, absoluteX);
      break; 
    case 0x40:
      cyclesCompleted = rti(cpu, bus);
      break;
    case 0x41:
      cyclesCompleted = eor(cpu, bus, indirectX);
      break;
    case 0x45:
      cyclesCompleted = eor(cpu, bus, zeroPage);
      break;
    case 0x46:
      cyclesCompleted = lsr(cpu, bus, zeroPage);
      break;
    case 0x48:
      cyclesCompleted = pha(cpu, bus);  
      break;
    case 0x49:
      cyclesCompleted = eor(cpu, bus, immediate);
      break;
    case 0x4a:
      cyclesCompleted = lsr(cpu, bus, accumulator);
      break;
    case 0x4c:
      cyclesCompleted = jmp(cpu, bus, absolute);
      break;
    case 0x4d:
      cyclesCompleted = eor(cpu, bus, absolute);
      break;
    case 0x4e:
      cyclesCompleted = lsr(cpu, bus, absolute);
      break;
    case 0x50:
      cyclesCompleted = bvc(cpu, bus);
      break;
    case 0x51:
      cyclesCompleted = eor(cpu, bus, indirectY);
      break;
    case 0x55:
      cyclesCompleted = eor(cpu, bus, zeroPageX);
      break;
    case 0x56:
      cyclesCompleted = lsr(cpu, bus, zeroPageX);
      break;
    case 0x58:
      cyclesCompleted = cli(cpu);
      break;
    case 0x59:
      cyclesCompleted = eor(cpu, bus, absoluteY);
      break;
    case 0x5d:
      cyclesCompleted = eor(cpu, bus, absoluteX);
      break;
    case 0x5e:
      cyclesCompleted = lsr(cpu, bus, absoluteX);
      break;
    case 0x60:
      cyclesCompleted = rts(cpu, bus);
      break;
    case 0x61:
      cyclesCompleted = adc(cpu, bus, indirectX);
      break;
    case 0x65:
      cyclesCompleted = adc(cpu, bus, zeroPage);
      break;
    case 0x66:
      cyclesCompleted = ror(cpu, bus, zeroPage);
      break;
    case 0x68:
      cyclesCompleted = pla(cpu, bus);
      break;
    case 0x69:
      cyclesCompleted = adc(cpu, bus, immediate);
      break;
    case 0x6a:
      cyclesCompleted = ror(cpu, bus, accumulator);
      break;
    case 0x6c:
      cyclesCompleted = jmp(cpu, bus, absoluteIndir);
      break;
    case 0x6d:
      cyclesCompleted = adc(cpu, bus, absolute);
      break;
    case 0x6e:
      cyclesCompleted = ror(cpu, bus, absolute);
      break;
    case 0x70:
      cyclesCompleted = bvs(cpu, bus);
      break;
    case 0x71:
      cyclesCompleted = adc(cpu, bus, indirectY);
      break;
    case 0x75:
      cyclesCompleted = adc(cpu, bus, zeroPageX);
      break;
    case 0x76:
      cyclesCompleted = ror(cpu, bus, zeroPageX);
      break;
    case 0x78:
      cyclesCompleted = sei(cpu, bus);
      break;
    case 0x79:
      cyclesCompleted = adc(cpu, bus, absoluteY);
      break;
    case 0x7d:
      cyclesCompleted = adc(cpu, bus, absoluteX);
      break;
    case 0x7e:
      cyclesCompleted = ror(cpu, bus, absoluteX);
      break;
    case 0x81:
      cyclesCompleted = sta(cpu, bus, indirectX);
      break;
    case 0x84:
      cyclesCompleted = sty(cpu, bus, zeroPage);
      break;
    case 0x85:
      cyclesCompleted = sta(cpu, bus, zeroPage);
      break;
    case 0x86:
      cyclesCompleted = stx(cpu, bus, zeroPage);
      break;
    case 0x88:
      cyclesCompleted = dey(cpu);
      break;
    case 0x8a:
      cyclesCompleted = txa(cpu);
      break;
    case 0x8c:
      cyclesCompleted = sty(cpu, bus, absolute);
      break;
    case 0x8d:
      cyclesCompleted = sta(cpu, bus, absolute);
      break;
    case 0x8e:
      cyclesCompleted = stx(cpu, bus, absolute);
      break;
    case 0x90:
      cyclesCompleted = bcc(cpu, bus);
      break;
    case 0x91:
      cyclesCompleted = sta(cpu, bus, indirectY);
      break;
    case 0x94:
      cyclesCompleted = sty(cpu, bus, zeroPageX);
      break;
    case 0x95:
      cyclesCompleted = sta(cpu, bus, zeroPageX);
      break;
    case 0x96:
      cyclesCompleted = stx(cpu, bus, zeroPageY);
      break;
    case 0x98:
      cyclesCompleted = tya(cpu);
      break;
    case 0x99:
      cyclesCompleted = sta(cpu, bus, absoluteY);
      break;
    case 0x9a:
      cyclesCompleted = txs(cpu);
      break;
    case 0x9d:
      cyclesCompleted = sta(cpu, bus, absoluteX);
      break;
    case 0xa0:
      cyclesCompleted = ldy(cpu, bus, immediate);
      break;
    case 0xa1:
      cyclesCompleted = lda(cpu, bus, indirectX);
      break;
    case 0xa2:
      cyclesCompleted = ldx(cpu, bus, immediate);
      break;
    case 0xa4:
      cyclesCompleted = ldy(cpu, bus, zeroPage);
      break;
    case 0xa5:
      cyclesCompleted = lda(cpu, bus, zeroPage);
      break;
    case 0xa6:
      cyclesCompleted = ldx(cpu, bus, zeroPage);
      break;
    case 0xa8:
      cyclesCompleted = tay(cpu);
      break;
    case 0xa9:
      cyclesCompleted = lda(cpu, bus, immediate);
      break;
    case 0xaa:
      cyclesCompleted = tax(cpu);
      break;
    case 0xac:
      cyclesCompleted = ldy(cpu, bus, absolute);
      break;
    case 0xad:
      cyclesCompleted = lda(cpu, bus, absolute);
      break;
    case 0xae:
      cyclesCompleted = ldx(cpu, bus, absolute);
      break;
    case 0xb0:
      cyclesCompleted = bcs(cpu, bus);
      break;
    case 0xb1:
      cyclesCompleted = lda(cpu, bus, indirectY);
      break;
    case 0xb4:
      cyclesCompleted = ldy(cpu, bus, zeroPageX);
      break;
    case 0xb5:
      cyclesCompleted = lda(cpu, bus, zeroPageX);
      break;
    case 0xb6:
      cyclesCompleted = ldx(cpu, bus, zeroPageY);
      break;
    case 0xb8:
      cyclesCompleted = clv(cpu);
      break;
    case 0xb9:
      cyclesCompleted = lda(cpu, bus, absoluteY);
      break;
    case 0xba:
      cyclesCompleted = tsx(cpu);
      break;
    case 0xbc:
      cyclesCompleted = ldy(cpu, bus, absoluteX);
      break;
    case 0xbd:
      cyclesCompleted = lda(cpu, bus, absoluteX);
      break;
    case 0xbe:
      cyclesCompleted = ldx(cpu, bus, absoluteY);
      break;
    case 0xc0:
      cyclesCompleted = cpy(cpu, bus, immediate);
      break;
    case 0xc1:
      cyclesCompleted = cmp(cpu, bus, indirectX);
      break;
    case 0xc4:
      cyclesCompleted = cpy(cpu, bus, zeroPage);
      break;
    case 0xc5:
      cyclesCompleted = cmp(cpu, bus, zeroPage);
      break;
    case 0xc6:
      cyclesCompleted = dec(cpu, bus, zeroPage);
      break;
    case 0xc8:
      cyclesCompleted = iny(cpu);
      break;
    case 0xc9:
      cyclesCompleted = cmp(cpu, bus, immediate);
      break;
    case 0xca:
      cyclesCompleted = dex(cpu, bus);
      break;
    case 0xcc:
      cyclesCompleted = cpy(cpu, bus, absolute);
      break;
    case 0xcd:
      cyclesCompleted = cmp(cpu, bus, absolute);
      break;
    case 0xce:
      cyclesCompleted = dec(cpu, bus, absolute);
      break;
    case 0xd0:
      cyclesCompleted = bne(cpu, bus);
      break;
    case 0xd1:
      cyclesCompleted = cmp(cpu, bus, indirectY);
      break;
    case 0xd5:
      cyclesCompleted = cmp(cpu, bus, zeroPageX);
      break;
    case 0xd6:
      cyclesCompleted = dec(cpu, bus, zeroPageX);
      break;
    case 0xd8:
      cyclesCompleted = cld(cpu);
      break;
    case 0xd9:
      cyclesCompleted = cmp(cpu, bus, absoluteY);
      break;
    case 0xdd:
      cyclesCompleted = cmp(cpu, bus, absoluteX);
      break;
    case 0xde:
      cyclesCompleted = dec(cpu, bus, absoluteX);
      break;
    case 0xe0:
      cyclesCompleted = cpx(cpu, bus, immediate);
      break;
    case 0xe1:
      cyclesCompleted = sbc(cpu, bus, indirectX);
      break;
    case 0xe4:
      cyclesCompleted = cpx(cpu, bus, zeroPage);
      break;
    case 0xe5:
      cyclesCompleted = sbc(cpu, bus, zeroPage);
      break;
    case 0xe6:
      cyclesCompleted = inc(cpu, bus, zeroPage);
      break;
    case 0xe8:
      cyclesCompleted = inx(cpu);
      break;
    case 0xe9:
      cyclesCompleted = sbc(cpu, bus, immediate);
      break;
    case 0xea:
      cyclesCompleted = nop(cpu);
      break;
    case 0xec:
      cyclesCompleted = cpx(cpu, bus, absolute);
      break;
    case 0xed:
      cyclesCompleted = sbc(cpu, bus, absolute);
      break;
    case 0xee:
      cyclesCompleted = inc(cpu, bus, absolute);
      break;
    case 0xf0:
      cyclesCompleted = beq(cpu, bus);
      break;
    case 0xf1:
      cyclesCompleted = sbc(cpu, bus, indirectY);
      break;
    case 0xf5:
      cyclesCompleted = sbc(cpu, bus, zeroPageX);
      break;
    case 0xf6:
      cyclesCompleted = inc(cpu, bus, zeroPageX);
      break;
    case 0xf8:
      // this function sets the Decimal flag, but has no function since
      // decimal mode doesn't exist on the nes
      cyclesCompleted = sed(cpu);
      break;
    case 0xf9:
      cyclesCompleted = sbc(cpu, bus, absoluteY);
      break;
    case 0xfd:
      cyclesCompleted = sbc(cpu, bus, absoluteX);
      break;
    case 0xfe:
      cyclesCompleted = inc(cpu, bus, absoluteX);
      break;
    default:
      printf("illegal instruction: %d - 0x%x \n", oppCode, oppCode);

      break;
  } 
  
  return cyclesCompleted;

}



int adc(CPU* cpu, Bus* bus, AddrMode mode){
  

  // TODO: redo the checkVFlag function;
  // might have every function have their own implementation of it
  uint8_t value;
  uint8_t prevA;
  uint8_t prevValue = 0;
  uint8_t prevBit7;
  uint8_t prevcpupf = cpu->pf;
  int cf;
  int carry = 0;
  int bit1 = 0;
  int bit2 = 0;
  int c6;
  int m7;
  int n7;


  value = addressModeDecode(cpu, bus, mode);
  prevA = cpu->a; 
  cpu->a += value;
  cpu->a = cpu->a + getBit(cpu->pf, C);
  //printf("Value from memory: %d \n", value);
  //printf("cpu->a: %d \n", prevA);

  // NV-BDIZC

  checkVFlag(cpu, value, prevA, ADD);
  checkCFlag(cpu, value, prevA, ADD);
  checkZFlag(cpu, cpu->a);
  //checkVFlag(cpu, cpu->a, value);
  //if(getBit(value, 7) != getBit(cpu->a, 7)){
  //if(((int8_t)cpu->a) < ((int8_t)prevA)){
  //  cpu->pf = setBit(cpu->pf, V);
  //} else {
  //  cpu->pf = clearBit(cpu->pf, V);
  //}
  
  /*
  for(int i = 0; i <= 6; ++i){
    bit1 = getBit(value, i) >> i; 
    bit2 = getBit(prevA, i) >> i; 
    printf("iteration %d \n", i);
    printf("bit1 %d \n", bit1);
    printf("bit2 %d \n", bit2);
    printf("carry %d \n", carry);
    printf("cpu->pf C flag: %d \n", getBit(prevcpupf, C));
    printf("cpu->pf %d \n", prevcpupf);
    if((i == 0) && (getBit(prevcpupf, C) == 1)){
      carry = 1;
    } 

    printf("Iteration %d \n", i);
    printf("\t carry %d \n", carry);
    printf("\t bit1 %d \n", bit1);
    printf("\t bit2 %d \n", bit2);

    if(bit1 + bit2 + carry >= 2){
      carry = 1;
    } else {
      carry = 0;
    }
  }

  c6 = carry;
  m7 = getBit(value, 7) >> 7;
  n7 = getBit(prevA, 7) >> 7;
  printf("c6 = %d \n", c6);
  printf("m7 = %d \n", m7);
  printf("n7 = %d \n", n7);
  
  cf = (!m7 & !n7 & c6) | (m7 & n7 & !c6);
  cf != 1 ? (cpu->pf = setBit(cpu->pf, V)) : (cpu->pf = clearBit(cpu->pf, V));
  if(cf == 1){
    cpu->pf = setBit(cpu->pf, V);
  } else {
    cpu->pf = clearBit(cpu->pf, V);
  }
  */
  checkNFlag(cpu, cpu->a);
  cpu->pc++;
  switch(mode){
    case immediate:
      return 2;
    case zeroPage:
      return 3;
    case zeroPageX:
      return 4;
    case absolute:
      return 4;
    case absoluteX:
      return pageFlag == 1 ? 5 : 4;
    case absoluteY:
      return pageFlag == 1 ? 5 : 4;
    case indirectX:
      return 6;
    case indirectY:
      return pageFlag == 1 ? 6 : 5;
    
  }

}


int and(CPU* cpu, Bus* bus, AddrMode mode){ 
    uint8_t value;
    value = addressModeDecode(cpu, bus, mode);
    cpu->a = cpu->a & value;

    checkNFlag(cpu, cpu->a);
    checkZFlag(cpu, cpu->a);
  
  cpu->pc++;
  switch(mode){
    case immediate:
      return 2;
    case zeroPage:
      return 3;
    case zeroPageX:
      return 4;
    case absolute:
      return 4;
    case absoluteX:
      return pageFlag == 1 ? 5 : 4;
    case absoluteY:
      return pageFlag == 1 ? 5 : 4;
    case indirectX:
      return 6;
    case indirectY:
      return pageFlag == 1 ? 6 : 5;
    
  }
}


int asl(CPU* cpu, Bus* bus, AddrMode mode) {
    uint8_t value;
    uint8_t prevValue;
    uint8_t pc = cpu->pc;
    value = addressModeDecode(cpu, bus, mode);
    // sets the carry bit to whatever the 7th position of the 
    // a register was, before the shift left occurs
    prevValue = value;
    value = value << 1;
    //printf("asl: %d \n", value);
    checkZFlag(cpu, value);
    checkNFlag(cpu, value);
    checkCFlag(cpu, value, prevValue, SHIFTL);
    //printf("cpu->pc at decode %d \n", cpu->pc);
    addressModeDecodeWrite(value, cpu, bus, mode);
    cpu->pc = cpu->pc + 1;
    switch(mode){
      case accumulator:
        return 2;
      case zeroPage:
        return 5;
      case zeroPageX:
        return 6;
      case absolute:
        return 6;
      case absoluteX:
        return 7;
    }
}


int bcc(CPU* cpu, Bus* bus){
  int8_t offset;
  uint16_t page;
  offset = addressModeDecode(cpu, bus, relative);
  page = cpu->pc & 0xff00;
  if(!getBit(cpu->pf, C)){
    cpu->pc += offset;
  }
  cpu->pc++;
  if(page == (cpu->pc & 0xff00)){
    return 3;
  } else if (page != (cpu->pc & 0xff00)) {
    return 4;
  }

  return 2;
}


int bcs(CPU* cpu, Bus* bus){
  int8_t offset;
  uint16_t page;

  offset = addressModeDecode(cpu, bus, relative);
  if(getBit(cpu->pf, C)){
    cpu->pc += offset;
  }

  cpu->pc++;
  if(page == (cpu->pc & 0xff00)){
    return 3;
  } else if (page != (cpu->pc & 0xff00)) {
    return 4;
  }

  return 2;
}

int beq(CPU* cpu, Bus* bus){
  int8_t offset;
  uint16_t page;

  offset = addressModeDecode(cpu, bus, relative);
  if(getBit(cpu->pf, Z) != 0){
    cpu->pc += offset;
  }
  cpu->pc++;
  if(page == (cpu->pc & 0xff00)){
    return 3;
  } else if (page != (cpu->pc & 0xff00)) {
    return 4;
  }

  return 2;

}


int bit(CPU* cpu, Bus* bus, AddrMode mode){

  uint8_t value = addressModeDecode(cpu, bus, mode);
  uint8_t prevValue = value;
    value = value & cpu->a;
    checkNFlag(cpu, prevValue);
    checkZFlag(cpu, value);


  // we are checking for the V flag here since the BIT instruction affects
  // the flag differently from other instructions, and thus our checkVFlag() function
  // is not suitable to use.
  //
  if(getBit(prevValue ,6) != 0){
    cpu->pf = setBit(cpu->pf, V);
  } else {
    cpu->pf = clearBit(cpu->pf, V);
  }
  cpu->pc++;
  switch(mode){
    case absolute:
      return 3;
    case zeroPage:
      return 4;
  }
  
}

int bmi(CPU* cpu, Bus* bus){
  int8_t offset;
  int cycles = 2;

  uint16_t page = cpu->pc & 0xff00;
  offset = addressModeDecode(cpu, bus, relative);
  if(getBit(cpu->pf, N) != 0){
    cpu->pc += offset;
    cycles += 1;
  }
  if(page == (cpu->pc & 0xff00)){
    cycles += 2;
  }
  cpu->pc++;
  return cycles;

}

int bne(CPU* cpu, Bus* bus){
  int8_t offset;
  uint16_t page;
  int cycles = 2;

  offset = addressModeDecode(cpu, bus, relative);

  if(getBit(cpu->pf, Z) == 0){
    cpu->pc += offset;
    cycles = 3;
  }
  if(page == (cpu->pc & 0xff00)){
    cycles += 2;
  }
  cpu->pc++;
  return cycles;


}

int bpl(CPU* cpu, Bus* bus){
  int8_t offset;
  uint16_t page;

  int cycles = 2;
  offset = (int8_t)addressModeDecode(cpu, bus, relative);
  if(!getBit(cpu->pf, N)){
    cpu->pc += offset;
  }

  if(getBit(cpu->pf, N) != 0){
    cycles += 1;
  }
  if(page == (cpu->pc & 0xff00)){
    cycles += 2;
  }
  cpu->pc++;
  return cycles;

}


int brki(CPU* cpu, Bus* bus){

  // break flag set to be prepared when pushed onto the stack
  cpu->pf = setBit(cpu->pf, 4);

 
  uint16_t temp;

  // push the msb and lsb of the program counter+2 onto the stack
  temp = cpu->pc + 2;
  pushStack(cpu, bus, (uint8_t)((temp & 0xff00) >> 8));
  pushStack(cpu, bus, (uint8_t)(temp & 0x00ff));
  //printf("Push Stack: %x \n", (uint8_t)((temp & 0xff00) >> 8));

  // pushes the processor flags onto the stack
  pushStack(cpu, bus, cpu->pf);

  // sets the interupt disable flag
  cpu->pf = setBit(cpu->pf, 2);


  // break flag is now cleared because it only exists within the stack
  cpu->pf = clearBit(cpu->pf, 4);


  cpu->pc = readBus(bus, 0xfffe);
  temp = (uint16_t)readBus(bus, 0xffff);
  temp = temp << 8;
  cpu->pc = cpu->pc | temp;
  //printf("Setting pc to %d \n", cpu->pc);

  return 7;


 


}

int bvc(CPU* cpu, Bus* bus){
  int8_t offset;
  uint16_t page;
  int cycles = 2;
  offset = addressModeDecode(cpu, bus, relative);
  if(!getBit(cpu->pf, V)){
    cpu->pc += offset;
  }

  if(!getBit(cpu->pf, V)){
    cycles += 1;
  }
  if(page == (cpu->pc & 0xff00)){
    cycles += 2;
  }
  cpu->pc++;
  return cycles;
}

int bvs(CPU* cpu, Bus* bus){
  int8_t offset;
  uint16_t page;
  int cycles = 2;
  offset = addressModeDecode(cpu, bus, relative);
  //uint8_t cpupclowbyte = cpu->pc & 0xff;

  if(getBit(cpu->pf, V)){
    cpu->pc += offset;
  }
  //cpu->pc = (cpu->pc & 0xff00) | cpupclowbyte;
  //
  if(getBit(cpu->pf, V)){
    cycles += 1;
  }
  if(page == (cpu->pc & 0xff00)){
    cycles += 2;
  }
  cpu->pc++;
  return cycles;
}

int clc(CPU* cpu){
  cpu->pf = clearBit(cpu->pf, C);
  cpu->pc++;
  return 2;
}


int plp(CPU* cpu, Bus* bus){
  cpu->pf = popStack(cpu, bus);
  cpu->pf = setBit(cpu->pf, 5);
  cpu->pf = clearBit(cpu->pf, 4);
  cpu->pc++;
  return 4;
}


int cld(CPU* cpu){
  cpu->pf = clearBit(cpu->pf, D);
  cpu->pc++;
  return 2;
}


int cli(CPU* cpu){
  cpu->pf = clearBit(cpu->pf, I);
  cpu->pc++;
  return 2;
}

int clv(CPU* cpu){
  cpu->pf = clearBit(cpu->pf, V);
  cpu->pc++;
  return 2;
}

int cmp(CPU* cpu, Bus* bus, AddrMode mode){
  uint8_t value;
  value = addressModeDecode(cpu, bus, mode);

  //checkCFlag(cpu, cpu->a, value, SUB);
  if(value <= cpu->a){
    cpu->pf = setBit(cpu->pf, C);
  } else {
    cpu->pf = clearBit(cpu->pf, C);
  }
  value = cpu->a - value;
  //printf("value: %d \n", value);
  //printf("accumlator: %d \n", cpu->a);
  checkZFlag(cpu, value);
  checkNFlag(cpu, value);

  cpu->pc++;
  
  switch(mode){
    case immediate:
      return 2;
    case zeroPage:
      return 3;
    case zeroPageX:
      return 4;
    case absolute:
      return 4;
    case absoluteX:
      return pageFlag == 1 ? 5 : 4;
    case absoluteY:
      return pageFlag == 1 ? 5 : 4;
    case indirectX:
      return 6;
    case indirectY:
      return pageFlag == 1 ? 6 : 5;

  }

}

int cpx(CPU* cpu, Bus* bus, AddrMode mode){

  uint8_t value = addressModeDecode(cpu, bus, mode);
  //checkCFlag(cpu, cpu->x, value, SUB);
  if(value <= cpu->x){
    cpu->pf = setBit(cpu->pf, C);
  } else {
    cpu->pf = clearBit(cpu->pf, C);
  }
  checkNFlag(cpu, cpu->x - value);
  checkZFlag(cpu, cpu->x - value);
  cpu->pc++;
  switch(mode){
    case immediate:
      return 2;
    case zeroPage:
      return 3;
    case absolute:
      return 4;
  }
  
}

int cpy(CPU* cpu, Bus* bus, AddrMode mode){
  uint8_t value = addressModeDecode(cpu, bus, mode);
  //checkCFlag(cpu, cpu->y, value, SUB);
  if(cpu->y >= value){
    cpu->pf = setBit(cpu->pf, C);
  } else {
    cpu->pf = clearBit(cpu->pf, C);
  }
  checkNFlag(cpu, cpu->y - value);
  checkZFlag(cpu, cpu->y - value);
  cpu->pc++;
  switch(mode){
    case immediate:
      return 2;
    case zeroPage:
      return 3;
    case absolute:
      return 4;
  }
  

}

int dec(CPU* cpu, Bus* bus, AddrMode mode){
  uint8_t value = addressModeDecode(cpu, bus, mode);
  value = value - 1;
  checkNFlag(cpu, value);
  checkZFlag(cpu, value);
  addressModeDecodeWrite(value, cpu, bus, mode);
  cpu->pc++;
  switch(mode){
    case zeroPage:
      return 5;
    case zeroPageX:
      return 6;
    case absolute:
      return 6;
    case absoluteX:
      return 7;
  }

}



int dex(CPU* cpu, Bus* bus){
  cpu->x--;
  checkNFlag(cpu, cpu->x);
  checkZFlag(cpu, cpu->x);
  cpu->pc++;
  return 2;
}

int dey(CPU* cpu){
  cpu->y--;
  checkNFlag(cpu, cpu->y);
  checkZFlag(cpu, cpu->y);
  cpu->pc++;
  return 2;
}

int eor(CPU* cpu, Bus* bus, AddrMode mode){
  uint8_t value = addressModeDecode(cpu, bus, mode);
  cpu->a = cpu->a ^ value;
  checkZFlag(cpu, cpu->a);
  checkNFlag(cpu, cpu->a);
  cpu->pc++;
  switch(mode){

    case immediate:
     return 2;
    case zeroPage:
     return 3;
    case zeroPageX:
     return 4;
    case absolute:
     return 4;
    case absoluteX:
     return pageFlag == 1 ? 5 : 4;
    case absoluteY:
     return pageFlag == 1 ? 5 : 4;
    case indirectX:
     return 6;
    case indirectY:
     return pageFlag == 1 ? 6 : 5;

  }

}

int inc(CPU* cpu, Bus* bus, AddrMode mode){
  uint8_t value = addressModeDecode(cpu, bus, mode);
  uint8_t prevValue = value;
  value++;
  checkNFlag(cpu, value);
  checkZFlag(cpu, value);
  addressModeDecodeWrite(value, cpu, bus, mode);
  cpu->pc++;
  switch(mode){
    case zeroPage:
      return 5;
    case zeroPageX:
      return 6;
    case absolute:
      return 6;
    case absoluteX:
      return 7;
  }

}


int inx(CPU* cpu){
  cpu->x++;
  checkZFlag(cpu, cpu->x);
  checkNFlag(cpu, cpu->x);
  cpu->pc++;
  return 2;


}

int iny(CPU* cpu){
  cpu->y++;
  checkZFlag(cpu, cpu->y);
  checkNFlag(cpu, cpu->y);
  cpu->pc++;
  return 2;

}

// jmp handles it's own address mode decoding, since addressModeDecode is unable
// return a 16 bit value
// also Absolute Indirect is only used in JMP.
int jmp(CPU* cpu, Bus* bus, AddrMode mode){
  uint8_t lowByte;
  uint16_t highByte, addr;

  lowByte = readBus(bus, ++cpu->pc);
  highByte = readBus(bus, ++cpu->pc);
  //printf("lowByte %d \n", lowByte);
  //printf("highByte %d \n", highByte);

  addr = (highByte << 8) | (uint16_t)lowByte;
  if(mode == absolute){
    //cpu->pc = ((uint16_t)readBus(bus, addr + 1)) << 8 | readBus(bus, addr);
    cpu->pc = addr;
  } else if(mode == absoluteIndir){
    cpu->pc = readBus(bus, addr) | (readBus(bus, (lowByte = lowByte + 1) | (highByte << 8)) << 8);
    
  }
  if(mode == absolute){
    return 3;
  } else {
    return 5;
  }
  //printf("addr: %x \n", addr);
  //printf("cpu->pc: %x \n", cpu->pc);
}


// jsr - jump to subroutine
// jumps to new address while pushing the contents of the program counter 
// onto the stack.
int jsr(CPU* cpu, Bus* bus){
  uint8_t lowByte;
  uint8_t highByte;
  uint16_t addr;
  lowByte = readBus(bus, ++cpu->pc);
  highByte = readBus(bus, ++cpu->pc);


  //printf("Current pc: %x \n", cpu->pc);
  //printf("Pushing %x \n", (uint8_t)cpu->pc);
  //printf("Pushing %x \n", (uint8_t)(cpu->pc >> 8));
  pushStack(cpu, bus, (uint8_t)(cpu->pc >> 8));
  pushStack(cpu, bus, (uint8_t)(cpu->pc & 0xff));
  cpu->pc = ((uint16_t)highByte) << 8;
  cpu->pc = cpu->pc | (uint16_t)lowByte;
  return 6;
  //printf("cpu->pc: %x \n", cpu->pc);
}


int lda(CPU* cpu, Bus* bus, AddrMode mode){
  uint8_t value = addressModeDecode(cpu, bus, mode); 
  //printf("lda value: %x \n", value);
  cpu->a = value;
  checkZFlag(cpu, cpu->a);
  checkNFlag(cpu, cpu->a);
  cpu->pc++;
  switch(mode){
    case immediate:
      return 2;
    case zeroPage:
      return 3;
    case zeroPageX:
      return 4;
    case absolute:
      return 4;
    case absoluteX:
      return pageFlag == 1 ? 5 : 4;
    case absoluteY:
      return pageFlag == 1 ? 5 : 4;
    case indirectX:
      return 6;
    case indirectY:
      return pageFlag == 1 ? 6 : 5;
  }
}

int ldx(CPU* cpu, Bus* bus, AddrMode mode){
  uint8_t value = addressModeDecode(cpu, bus, mode); 
  cpu->x = value;
  checkZFlag(cpu, cpu->x);
  checkNFlag(cpu, cpu->x);
  cpu->pc++;
  switch(mode){

    case immediate:
      return 2;
    case zeroPage:
      return 3;
    case zeroPageX:
      return 4;
    case absolute:
      return 4;
    case absoluteY:
      return pageFlag == 1 ? 5 : 4;

  }
}


int ldy(CPU* cpu, Bus* bus, AddrMode mode){
  uint8_t value = addressModeDecode(cpu, bus, mode); 
  cpu->y = value;
  checkZFlag(cpu, cpu->y);
  checkNFlag(cpu, cpu->y);
  cpu->pc++;
  
   switch(mode){

    case immediate:
      return 2;
    case zeroPage:
      return 3;
    case zeroPageX:
      return 4;
    case absolute:
      return 4;
    case absoluteY:
      return pageFlag == 1 ? 5 : 4;

  } 
  
}

int lsr(CPU* cpu, Bus* bus, AddrMode mode){
  uint8_t value = addressModeDecode(cpu, bus, mode);
  uint8_t prevValue = value;
  value = value >> 1;
  checkNFlag(cpu, value);
  checkZFlag(cpu, value);
  checkCFlag(cpu, value, prevValue, ROTATER);
  addressModeDecodeWrite(value, cpu, bus, mode);
  cpu->pc++;
  switch(mode){

    case accumulator:
      return 2;
    case zeroPage:
      return 5;
    case zeroPageX:
      return 6;
    case absolute:
      return 6;
    case absoluteX:
      return 7;

  }
}

int nop(CPU* cpu){
  cpu->pc++;
  return 2;
}

int ora(CPU* cpu, Bus* bus, AddrMode mode){
  uint8_t value = addressModeDecode(cpu, bus, mode);
  //printf("Doing ORA operation on %x \n", value);
  cpu->a = cpu->a | value;
  checkNFlag(cpu, cpu->a);
  checkZFlag(cpu, cpu->a);
  cpu->pc++;
  switch(mode){
    case immediate:
      return 2;
    case zeroPage:
      return 3;
    case zeroPageX:
      return 4;
    case zeroPageY:
      return 4;
    case absolute:
      return 4;
    case absoluteX:
      return pageFlag == 1 ? 5 : 4;
    case absoluteY:
      return pageFlag == 1 ? 5 : 4;
    case indirectX:
      return 6;
    case indirectY:
      return pageFlag == 1 ? 6 : 5;
  }
}

int pha(CPU* cpu, Bus* bus){
  pushStack(cpu, bus, cpu->a);
  cpu->pc++;
  return 3;
}


int php(CPU* cpu, Bus* bus){
  uint8_t val;
  val = setBit(cpu->pf, B);
  pushStack(cpu, bus, val);  
  cpu->pc++;
  return 3;
}


int pla(CPU* cpu, Bus* bus){
  cpu->a = popStack(cpu, bus);
  //printf("cpu->pf in pla: %d \n", cpu->pf);
  checkNFlag(cpu, cpu->a);
  checkZFlag(cpu, cpu->a);
  cpu->pc++;
  return 4;
}


int rol(CPU* cpu, Bus* bus, AddrMode mode){
  uint8_t value = addressModeDecode(cpu, bus, mode);
  uint8_t prevValue = value;

  // sets the C Flag as bit 7 of the input 

  
  value = value << 1;

  // sets bit 0 as the input carry (after the operation as taken place)
  if(getBit(cpu->pf, 0) == 0){
    value = clearBit(value, 0); 
  } else {
    value = setBit(value, 0);
  }

  checkCFlag(cpu, value, prevValue, ROTATEL);  
  checkZFlag(cpu, value);
  checkNFlag(cpu, value);
  addressModeDecodeWrite(value, cpu, bus, mode);
  cpu->pc++;
  switch(mode){
    case accumulator:
      return 2;
    case zeroPage:
      return 5;
    case zeroPageX:
      return 6;
    case absolute:
      return 6;
    case absoluteX:
      return 7;

  }

}


int ror(CPU* cpu, Bus* bus, AddrMode mode){
  uint8_t value = addressModeDecode(cpu, bus, mode); 
  uint8_t prevValue = value;


  //printf("value before ror: %d \n", value);
  value = value >> 1;
  //printf("value after ror: %d \n", value);

  // sets bit 0 to the carry flag of the previous operation
  if(getBit(cpu->pf, C) == 0){
    value = clearBit(value, 7);
  } else {
    value = setBit(value, 7);
  }

  checkCFlag(cpu, value, prevValue, ROTATER);  
  checkZFlag(cpu, value);
  checkNFlag(cpu, value);
  addressModeDecodeWrite(value, cpu, bus, mode);
  cpu->pc++;
  switch(mode){
    case accumulator:
      return 2;
    case zeroPage:
      return 5;
    case zeroPageX:
      return 6;
    case absolute:
      return 6;
    case absoluteX:
      return 7;

  }

}

int rti(CPU* cpu, Bus* bus){



  cpu->pf = popStack(cpu, bus);
  cpu->pf = setBit(cpu->pf, U);
  cpu->pc = (uint16_t)popStack(cpu, bus);
  cpu->pc = (cpu->pc | (((uint16_t)popStack(cpu, bus)) << 8));
  //printf("cpu->pc after popping in rti: %x \n", cpu->pc);


  // clears the brk flag
  // 
  cpu->pf = clearBit(cpu->pf, 4);

  return 6; 



}

int rts(CPU* cpu, Bus* bus){
  cpu->pc = (uint16_t) popStack(cpu, bus);
  cpu->pc += (uint16_t) popStack(cpu, bus) << 8;
  cpu->pc++;
  return 6;
}

int sec(CPU* cpu, Bus* bus){
  cpu->pf = setBit(cpu->pf, C);
  cpu->pc++;
  return 2;
}

int sed(CPU* cpu){
  cpu->pf = setBit(cpu->pf, D);
  cpu->pc++;
  return 2;

}
int sei(CPU* cpu, Bus* bus){
  cpu->pf = setBit(cpu->pf, I);
  cpu->pc++;
  return 2;
}



int sbc(CPU* cpu, Bus* bus, AddrMode mode){
  uint8_t value = addressModeDecode(cpu, bus, mode);
  uint8_t prevA = cpu->a;
  uint16_t temp = 0;

  // getting one's complement
  value = ~value;
  cpu->a += value;
  cpu->a += getBit(cpu->pf, C);
  temp = value + prevA + getBit(cpu->pf, C);
  
  //printf("sbc: value - %d ; prevA - %d \n", value, prevA);

  checkVFlag(cpu, value, prevA, SUB);
  checkZFlag(cpu, cpu->a);
  checkNFlag(cpu, cpu->a);

  
  //printf("temp: %d \n", temp);
  if(temp >= 256){
    //puts("setting C flag \n");
    cpu->pf = setBit(cpu->pf, C);
  } else {
    //puts("clearing C flag \n");
    cpu->pf = clearBit(cpu->pf, C);
  }

  cpu->pc++;
  switch(mode){

    case immediate:
      return 2;
    case zeroPage:
      return 3;
    case zeroPageX:
      return 4;
    case absolute:
      return 4;
    case absoluteX:
      return pageFlag == 1 ? 5 : 4;
    case absoluteY:
      return pageFlag == 1 ? 5 : 4;
    case indirectX:
      return 6;
    case indirectY:
      return pageFlag == 1 ? 6 : 5;

  }


}



// STA - store accumulator in memory
int sta(CPU* cpu, Bus* bus, AddrMode mode){
  switch(mode){
    case absolute:
    case absoluteY:
    case absoluteX:
      cpu->pc++;
      cpu->pc++;
      break;
    case zeroPage:
    case indirectX:
    case indirectY:
    case zeroPageX:
      cpu->pc++;
      break;
  }
  // if(mode == absolute || mode == absoluteY){
  //   cpu->pc++;
  //   cpu->pc++;
  // } else if(mode == zeroPage || mode == indirectX || mode == indirectY || mode == zeroPageX){
  //   cpu->pc++;
  // }
  addressModeDecodeWrite(cpu->a, cpu, bus, mode);
  cpu->pc++;
  switch(mode){
    case zeroPage:
      return 3;
    case zeroPageX:
      return 4;
    case absolute:
      return 4;
    case absoluteX:
      return 5;
    case absoluteY:
      return 5;
    case indirectX:
      return 6;
    case indirectY:
      return 6;
  }
  
}

int stx(CPU* cpu, Bus* bus, AddrMode mode){


  if(mode == absolute){
    cpu->pc++;
    cpu->pc++;
  } else if (mode == zeroPage | mode == zeroPageY){
    cpu->pc++;
  }

  addressModeDecodeWrite(cpu->x, cpu, bus, mode);
  cpu->pc++;
  switch(mode){
    case zeroPage:
      return 3;
    case zeroPageY:
      return 4;
    case absolute:
      return 4;

  }
}


int sty(CPU* cpu, Bus* bus, AddrMode mode){
  if(mode == absolute){
    cpu->pc++;
    cpu->pc++;
  } else if (mode == zeroPage || mode == zeroPageX){
    cpu->pc++;
  }
  addressModeDecodeWrite(cpu->y, cpu, bus, mode);
  cpu->pc++;
  switch(mode){
    case zeroPage:
      return 3;
    case zeroPageY:
      return 4;
    case absolute:
      return 4;

  }

}

int tax(CPU* cpu){
  cpu->x = cpu->a;
  checkNFlag(cpu, cpu->x);
  checkZFlag(cpu, cpu->x);
  cpu->pc++;
  return 2;
}


int tay(CPU* cpu){
  cpu->y = cpu->a;
  checkNFlag(cpu, cpu->y);
  checkZFlag(cpu, cpu->y);
  cpu->pc++;
  return 2;
}


int tsx(CPU* cpu){
  cpu->x = cpu->sp;
  checkNFlag(cpu, cpu->x);
  checkZFlag(cpu, cpu->x);
  cpu->pc++;
  return 2;
}

int txa(CPU* cpu){
  cpu->a = cpu->x;
  checkNFlag(cpu, cpu->a);
  checkZFlag(cpu, cpu->a);
  cpu->pc++;
}

int txs(CPU* cpu){
  cpu->sp = cpu->x;
  cpu->pc++;
  return 2;
}

int tya(CPU* cpu){
  cpu->a = cpu->y;
  checkNFlag(cpu, cpu->a);
  checkZFlag(cpu, cpu->a);
  cpu->pc++;
  return 2;
}




void addressModeDecodeWrite(uint8_t value, CPU* cpu, Bus* bus, AddrMode mode){


  // TODO: fix cpu->pc incrementing; some addressing modes increment the pc differently and thus
  // are incompatible with instructions that do not do an addressModeDecode first ex: stx, sty
  // 
  uint16_t lowByte, highByte;
  uint8_t zeroPageAddr;

  switch(mode){
    case absolute:
      lowByte = readBus(bus, cpu->pc - 1);
      highByte = readBus(bus, cpu->pc);
      writeBus(bus, (highByte << 8) + lowByte, value);
      return;
    case absoluteX:
      lowByte = readBus(bus, cpu->pc - 1);
      highByte = readBus(bus, cpu->pc);
      writeBus(bus, (highByte << 8) + lowByte + cpu->x, value);
      return;
    case absoluteY:
      lowByte = readBus(bus, cpu->pc - 1);
      highByte = readBus(bus, cpu->pc);
      writeBus(bus, (highByte << 8) + lowByte + cpu->y, value);
      return;
    case accumulator:
      cpu->a = value;
      return;
    case zeroPage:
      lowByte = readBus(bus, cpu->pc);
      writeBus(bus, lowByte & 0xff, value);
      return;
    case zeroPageX:
      // bitwise AND with 0xff so as to only get the lower 8 bits
      zeroPageAddr = readBus(bus, cpu->pc);
      zeroPageAddr = zeroPageAddr + cpu->x;
      writeBus(bus, zeroPageAddr, value);
      return;
    case zeroPageY:
      zeroPageAddr = readBus(bus, cpu->pc);
      writeBus(bus, zeroPageAddr = zeroPageAddr + cpu->y, value);
      return;
    case indirectX:
      // inner readBus function gets the bytes from the second byte of the instruction
      //
      // outer readBus function gets the low and high bytes, of which are in the
      // zero page and who's contents will yield our effective address
      lowByte = readBus(bus, readBus(bus, cpu->pc) + cpu->x & 0xff);
      highByte = readBus(bus, readBus(bus, cpu->pc) + cpu->x + 1 & 0xff);
      writeBus(bus, (highByte << 8) + lowByte, value);
      return;
    case indirectY:
      zeroPageAddr = readBus(bus, cpu->pc);
      lowByte = readBus(bus, zeroPageAddr);
      highByte = readBus(bus, ++zeroPageAddr);
      writeBus(bus, (highByte << 8) + (lowByte = lowByte + cpu->y), value);
      return;

  }
  



}


uint8_t addressModeDecode(CPU* cpu, Bus* bus, AddrMode mode){
  uint16_t lowByte, highByte, byte, addr;
  uint8_t zeroPageAddr;
  switch(mode){
    case immediate:
      return readBus(bus, ++cpu->pc);
    case accumulator:
      return cpu->a;
    case relative:
      return readBus(bus, ++cpu->pc);
      
    case absolute:
      lowByte = readBus(bus, ++cpu->pc);
      highByte = readBus(bus, ++cpu->pc);
      uint16_t currPage = cpu->pc & 0xff00;
      uint16_t newPage = ((highByte << 8) + lowByte) & 0xff00;
      if(currPage == newPage){
        pageFlag = 1;
      } else {
        pageFlag = 0;
      }
      return readBus(bus, (highByte << 8) + lowByte);

    case absoluteX:        
      lowByte = readBus(bus, ++cpu->pc); 
      highByte = readBus(bus, ++cpu->pc); 
      return readBus(bus, (highByte << 8) + lowByte + cpu->x);

    case absoluteY:
      lowByte = readBus(bus, ++cpu->pc); 
      highByte = readBus(bus, ++cpu->pc); 
      return readBus(bus, (highByte << 8) + lowByte + cpu->y);

    case zeroPage:
      lowByte = readBus(bus, ++cpu->pc);
      return readBus(bus, lowByte);

    case zeroPageX:
      zeroPageAddr = readBus(bus, ++cpu->pc);
      //printf("doing zeropagex with %d \n", cpu->pc);
      zeroPageAddr = zeroPageAddr + cpu->x;
      return readBus(bus, zeroPageAddr);
    
    case zeroPageY:
      zeroPageAddr = readBus(bus, ++cpu->pc);
      return readBus(bus, zeroPageAddr = zeroPageAddr + cpu->y);
      
    case indirectX:
      lowByte = readBus(bus, (uint8_t)(cpu->x + readBus(bus, ++cpu->pc))); 
      //printf("Reading low byte %d \n", readBus(bus, (uint8_t)(cpu->x + readBus(bus, cpu->pc))));

      highByte = readBus(bus, (uint8_t)(cpu->x + readBus(bus, cpu->pc) + 1)); 
      //printf("Reading low byte %d \n", lowByte);
      //printf("Reading high byte %d \n", highByte);
      //printf("Doing indirect-x \n");
      // reads zero page
      return readBus(bus, (highByte << 8) | lowByte);
      
    case indirectY:
      zeroPageAddr = readBus(bus, ++cpu->pc); 
      lowByte = readBus(bus, zeroPageAddr);
      highByte = readBus(bus, ++zeroPageAddr);
      return readBus(bus, ((highByte << 8) | lowByte) + cpu->y);
      
  } 



}


void pushStack(CPU* cpu, Bus* bus, uint8_t val){
  writeBus(bus, 0x0100 | ((uint16_t)cpu->sp), val);
  cpu->sp--;
}

uint8_t popStack(CPU* cpu, Bus* bus){
  uint8_t temp = readBus(bus, 0x0100 | ((uint16_t)(++cpu->sp)));
  return temp;
}

void checkNFlag(CPU* cpu, uint8_t val){
  if(getBit(val, N)){
    cpu->pf = setBit(cpu->pf, N);
  } else {
    cpu->pf = clearBit(cpu->pf, N);
  }
}


// how this function works is that it performs a bit-wise addition manually on the two
// operands and then checks M7, N7, and C6, and sees if they correspond with the overflow bit
// using the table from https://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
//
// M7 - 7th bit of first operand
// N7 - 7th bit of second operand
// C6 - carry out from 6th bit operation

void checkVFlag(CPU* cpu, uint8_t val, uint8_t prevVal, uint8_t operationFlag){
  


  uint8_t bit1;
  uint8_t bit2;
  uint8_t carry;
  uint8_t prevcpupf = cpu->pf;
  uint8_t c6;
  uint8_t c7;
  uint8_t m7;
  uint8_t n7;
  uint8_t cf = 0;

  if(operationFlag == SUB || operationFlag == ADD){
    cf = (~(prevVal ^ val) & (prevVal ^ cpu->a)) & 0x80;


    if(cf != 0){
      cpu->pf = setBit(cpu->pf, V);
    } else {
      cpu->pf = clearBit(cpu->pf, V);
    }
  }
  
}

// special checkVFlag function specifically made for the bit instruction since
// that instruction affects the V flag differently than other instructions
// input:
//   cpu - cpu in question 
//   val - value to test the V flag for (being the value at the memory location)
void checkVFlagForBit(CPU* cpu, uint8_t val){
  if(getBit(val,6) != 0){
    cpu->pf = setBit(val, V);
  } else {
    cpu->pf = clearBit(val, V);
  }


}

void checkZFlag(CPU* cpu, uint8_t val){
  if(val == 0){
    cpu->pf = setBit(cpu->pf, Z);
  } else {
    cpu->pf = clearBit(cpu->pf, Z);
  }
}

// operationFlag refers to whether an add, sub, ror, or rol operation occured 
// and will check the Carry Flag accordingly
//
// checkCFlag will perform a bitwise operation bit by bit, 
// and on the 7th-position or 1st-position bit, 
// it will check to see whether the carry was set
// and set the cpu flag to that
//
// operand1 - is the value after the operation was performed
// operand2 - is the value before the operation will be performed
void checkCFlag(CPU* cpu, uint8_t operand1, uint8_t operand2, uint8_t operationFlag){
  uint8_t bit1;
  uint8_t bit2;
  uint8_t carry = 0;


  // TODO: check to see how this function works (?)
  //

  if(operationFlag == ROTATEL || operationFlag == SHIFTL){
    // operand2 becomes value before the operation was performed
    // operand1 becomes value after the operation was performed
    //fputs("shiftl or rotatel \n", stdout);
    //printf("cpu->pf %d \n", cpu->pf);
    //if(getBit(operand2, 7) != 0){
    //  cpu->pf = setBit(cpu->pf, 0);
    //} else {
    //  cpu->pf = clearBit(cpu->pf, 0);
    //}
    cpu->pf = (getBit(operand2, 7) != 0 ? setBit(cpu->pf, C) : clearBit(cpu->pf, C));
    //printf("cpu->pf %d \n", cpu->pf);
    //printf("cpu->pf %d \n",  getBit(operand2, 7) == 0 ? setBit(cpu->pf, 7) : clearBit(cpu->pf, 7));
    return;
  } else if(operationFlag == ROTATER){
    if(getBit(operand2, 0) != 0){
      cpu->pf = setBit(cpu->pf, C);
    } else {
      cpu->pf = clearBit(cpu->pf, C);
    }
    //cpu->pf = setBit(cpu->pf, getBit(operand2, 0));
    return;
  } 

  /*else if(operationFlag == ADD){
    if((operand1 + operand2) <= 255){
      cpu->pf = clearBit(cpu->pf, C);
    } else {
      cpu->pf = setBit(cpu->pf, C);
    }
  }*/
     

  if(operationFlag == SHIFTL){
    // operand2 becomes value before the operation was performed
    // operand1 becomes value after the operation was performed


  }

  // find twos complement of a number
  if(operationFlag == SUB){
    //operand2 = ~operand2;
    //int8_t tempoper1 = (int8_t)operand1;  
    //int8_t tempoper2 = (int8_t)operand2;  
    //if(tempoper1 - tempoper2 < 0){
    //  
    //}
    
  }

  
  for(int i = 0; i <= 7; ++i){
    bit1 = getBit(operand1, i) >> i; 
    bit2 = getBit(operand2, i) >> i; 
    //printf("iteration %d \n", i);
    //printf("bit1 %d \n", bit1);
    //printf("bit2 %d \n", bit2);
    //printf("carry %d \n", carry);
    if(i == 0 && bit1 + bit2 + carry + getBit(cpu->pf, C) >= 2){
      carry = 1;
      continue;
    } 
    if(bit1 + bit2 + carry >= 2){
      carry = 1;
    } else {
      carry = 0;
    }
  }
  if(carry == 1){
    //printf("Setting carry! \n");
    cpu->pf = setBit(cpu->pf, C);
  } else {
    //printf("Clearing carry! \n");
    cpu->pf = clearBit(cpu->pf, C);
  }
}
