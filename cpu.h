#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "general.h"
#include "memory.h"

typedef struct _Bus Bus;

// U is used to denote the unused flag, always pushed as a one
enum flagBits {C, Z, I, D, B, U, V, N};

#define ADD 0
#define SUB 1
#define ROTATEL 2
#define ROTATER 3
#define SHIFTL 4

typedef enum {immediate, accumulator, absolute, absoluteX, absoluteY, absoluteIndir, 
  zeroPage, zeroPageX, zeroPageY, indirectX, indirectY, relative, indirect}AddrMode;


typedef struct _CPU {
  uint8_t a;
  uint8_t x; 
  uint8_t y; 

  uint8_t sp; 
  uint16_t pc; 
  uint16_t addrBus;
  uint8_t dataBus;
  int cycles;

  // processor flags
  uint8_t pf; 


  // used for the calculation of the carry flag
  uint8_t prevpf; 
  

  int nmiInterruptFlag;

}CPU;



uint8_t readBus(Bus*, uint16_t);
void writeBus(Bus*, uint16_t, uint8_t);

int execute(CPU*, Bus*, int);

// ** Global Variables **
//
// pageFlag is used in addressModeDecode to denote whether
// the resolved address crossing page boundaries
extern int pageFlag;

//void adc(CPU*, Bus*, uint16_t, uint8_t, addrMode);
//void and(CPU*, Bus*, uint16_t, uint8_t, addrMode); 
//void asl(Machine*, uint16_t, uint8_t, addrMode); 
void reset(CPU*, Bus*);
int nmi(CPU*, Bus*);
int irq(CPU*, Bus*);
int brki(CPU*, Bus*);

void triggerNmi(CPU*);

void checkForInterrupts(CPU*, Bus*);

int decodeAndExecute(CPU*, Bus*, uint8_t);


int adc(CPU*, Bus*, AddrMode);
int and(CPU*, Bus*, AddrMode);
int asl(CPU*, Bus*, AddrMode);
int bcc(CPU*, Bus*);
int bcs(CPU*, Bus*);
int beq(CPU*, Bus*);
int bit(CPU*, Bus*, AddrMode);
int bmi(CPU*, Bus*);
int bne(CPU*, Bus*);
int bpl(CPU*, Bus*);
int bvc(CPU*, Bus*);
int bvs(CPU*, Bus*);

int clc(CPU*);
int cld(CPU*);
int cli(CPU*);
int clv(CPU*);
int cmp(CPU*, Bus*, AddrMode);

int cpx(CPU*, Bus*, AddrMode);
int cpy(CPU*, Bus*, AddrMode);
int dec(CPU*, Bus*, AddrMode);
int dex(CPU*, Bus*);
int dey(CPU*);
int eor(CPU*, Bus*, AddrMode);
int inc(CPU*, Bus*, AddrMode);
int inx(CPU*);
int iny(CPU*);

int jmp(CPU*, Bus*, AddrMode);
int jsr(CPU*, Bus*);


int lda(CPU*, Bus*, AddrMode);
int ldx(CPU*, Bus*, AddrMode);
int ldy(CPU*, Bus*, AddrMode);
int lsr(CPU*, Bus*, AddrMode);
int nop(CPU*);

int ora(CPU*, Bus*, AddrMode);

int pha(CPU*, Bus*);
int php(CPU*, Bus*);
int pla(CPU*, Bus*);
int plp(CPU*, Bus*);

int rol(CPU*, Bus*, AddrMode);
int ror(CPU*, Bus*, AddrMode);
int rti(CPU*, Bus*);
int rts(CPU*, Bus*);

int sec(CPU*, Bus*);
int sei(CPU*, Bus*);
int sed(CPU*);

int sbc(CPU*, Bus*, AddrMode);
int sta(CPU*, Bus*, AddrMode);
int stx(CPU*, Bus*, AddrMode);
int sty(CPU*, Bus*, AddrMode);

int tax(CPU*);
int tay(CPU*);
int tsx(CPU*);
int txa(CPU*);
int txs(CPU*);
int tya(CPU*);

void checkNFlag(CPU*, uint8_t);
void checkVFlag(CPU*, uint8_t, uint8_t, uint8_t);
void checkZFlag(CPU*, uint8_t);
void checkCFlag(CPU*, uint8_t, uint8_t, uint8_t);

uint8_t addressModeDecode(CPU*, Bus*, AddrMode);
void addressModeDecodeWrite(uint8_t, CPU*, Bus*, AddrMode);

void pushStack(CPU*, Bus*, uint8_t);
uint8_t popStack(CPU*, Bus*);



//#endif
