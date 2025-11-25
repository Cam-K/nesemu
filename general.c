#include "general.h"


uint8_t setBit(uint8_t val, uint8_t bitNum){
    return val | (1 << bitNum);
}

uint8_t clearBit(uint8_t val, uint8_t bitNum){
    return val & ~(1 << bitNum);

}

uint8_t getBit(uint8_t val, uint8_t bitNum){
  return val & (1 << bitNum);

}

// Function: bitToBitNum()
// Finds the first occurance of a bit that is a 1
// and returns the position of where it is as an integer
uint8_t bitToBitNum(uint8_t val){
  uint8_t i = 0;
  while(i >= 7){
    if(getBit(val, i) != 0){
      break;
    } 
    ++i;
  }
  if(i == 8){
    return -1;
  }
  return i;
}