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


#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdint.h>
#include <bits/getopt_core.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cjson/cJSON.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cpu.h"
#include "memory.h"
#include "ppu.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include "general.h"

#define MAX_STR 128

// original resolution of Nintendo

// this includes the hblanking period as well
#define CPU_CYCLES_PER_SCANLINE 114

void parseTwoHexNums(char*, uint16_t*, uint16_t*);

void printHelp();
void interpreter(Bus*);




typedef struct procState {
  uint8_t x;
  uint8_t y;
  uint8_t a;
  uint16_t pc;
  uint8_t p;
  uint8_t s;
  uint8_t lastOpCode;

  // this serves as a flag to let the function know
  // if the data that's returned is bad 
  int errorFlag;

  // s is the stack pointer 
  // p is the status register



} processorState;

int jsonTester(char*, Bus*, processorState*);

// struct to encapsulate the results from a json test.
// needed for multithreaded tests
typedef struct _tr {

  // stores the last instruction executed
  uint8_t instruction;
  char name[MAX_STR];
  processorState state;
  processorState json;
  int passFlag;




} TestResults;

//
// struct used to pass arguments to a thread with 
// pthread_create that only accepts void* as arguments
typedef struct aft {

  Bus* bus;
  char file[MAX_STR];
  uint8_t opcodeToTest;
  TestResults* testResults;

} ArgsForThreads;

TestResults* jsonTesterParallel(char**, Bus*, int, int);
void startNes(char*, int);
void nesMainLoop(Bus*, SDL_Renderer*, SDL_Texture*, int);
void freeAndExit(Bus*);






int main(int argc, char* argv[]){

  

  uint8_t oppCode;



  // general purpose iterator variables
  uint16_t i;


  // variables related to file handling
  int fileSize;
  int fFlag = 0;
  int hFlag = 0;
  int nFlag = 0;
  int iFlag = 0;
  int sFlag = 0;
  int opt;
  int jFlag = 0;
  char fileDirectory[MAX_STR];
  char file[MAX_STR];
  char screenScaling[MAX_STR];
  screenScaling[0] = '\0';
  uint8_t* fileBuffer;


  FILE* fptr;
  printf("    nesemu  Copyright (C) 2026  Cameron Kelly \n This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'. \n This is free software, and you are welcome to redistribute it \n under certain conditions; type `show c' for details. \n");

  // parsing command line arguments
  if(argc > 1){
    while((opt = getopt(argc, argv, "fjhnis")) != -1)
    {
      switch(opt){
        case 'f':
          fFlag = 1;
          break;
        case 'j':
          // Json tester
          jFlag = 1;
          if(argv[optind] != NULL){
            strcpy(file, argv[optind]);
          }
          break;
        case 'n':
          // starts NES emulator
          nFlag = 1;
          if(argv[optind] != NULL){
            strcpy(file, argv[optind]);
          }
          break;
        case 'h':
          // prints help
          hFlag = 1;
          break;
          
        case 'i':
          // starts interpreter
          iFlag = 1;
          break;
        case 's':
          sFlag = 1;
          if(argv[optind] != NULL){
            strcpy(screenScaling, argv[optind]);
          }
          break;
          
      }
    } 
  } else {
    printHelp();
  }
  
  if(hFlag == 1){
    printHelp();
  }

  
  // start Tom Harte's tester
  if(jFlag == 1){
    Bus bus;
    initBus(&bus, 1);
    initMemStruct(&(bus.memArr[0]), 0xffff, Ram, TRUE);
    mapMemory(&bus, 0, 0x0000);
    printf("Entering Json Mode \n");
    if(jsonTester(file, &bus, NULL) == 1){
      printf("Passed all tests! \n");  
    }
        
    exit(1);

  
  }
      
 


  // Basic file handling
  // Loads file contents into Rom bank

  if(fFlag == 1){
    Bus bus;
    initMemStruct(&(bus.memArr[0]) , 0xbfff, Ram, TRUE);
    mapMemory(&bus, 0, 0x0000);

    fptr = fopen(fileDirectory, "r");
    if(fptr == NULL){
      printf("Cannot open file \n");
      exit(1);
    }
    fseek(fptr, 0L, SEEK_END);
    fileSize = ftell(fptr);
    rewind(fptr);
    printf("File size is %d bytes \n", fileSize);
    fileBuffer = (uint8_t*)calloc(fileSize, sizeof(uint8_t));
    for(i = 0; i < fileSize; ++i){
      (void)!fread(fileBuffer + i, 8, 1, fptr);   
      if(i == 0 || i == 1)
        printf("%x \n", fileBuffer[i]);
    }

    // dumping file into memory
    // TODO: file is not being dumped into memory properly (problem could be here or
    // in the readBus() function)

    initMemStruct(&(bus.memArr[1]), fileSize, Rom, TRUE);
    mapMemory(&bus, 1, 0xffff - fileSize);
    printf("%d \n", fileSize);
    printf("%d \n", bus.memArr[1].size);

    for(i = 0; i < fileSize; ++i){
      bus.memArr[1].contents[i] = fileBuffer[i];
      if(i == 0 || i == 1)
        printf("%x \n", bus.memArr[1].contents[i]);
    }
    interpreter(&bus);
  } 

  if(nFlag == 1){
    startNes(file, atoi(screenScaling));
  }
  
  // starts interpreter with no file
  // - empty chunk of 64k memory 
  if(iFlag == 1){
    Bus bus;
    printf("Starting interpreter \n");
    initBus(&bus, 1);
    initMemStruct(&(bus.memArr[0]) , 0xffff, Ram, TRUE);
    mapMemory(&bus, 0, 0x0000);
    

    interpreter(&bus);


  }

}


// converts a string containing two hex numbers delimited by a comma, to two 16 bit integers
void parseTwoHexNums(char* input, uint16_t* start, uint16_t* end){
  char firstString[MAX_STR];
  char secondString[MAX_STR];
  int i;
  for(i = 0; input[i] != ',' || input[i] == '\0'; ++i){
    firstString[i] = input[i]; 
  }
  firstString[i] = '\0';
  ++i;
  for(; input[i] != '\0'; ++i){
    secondString[i - strlen(firstString) - 1] = input[i]; 
  }
  secondString[i - strlen(firstString) - 1] = '\0';

  *start = (uint16_t)strtol(firstString, NULL, 16);
  *end = (uint16_t)strtol(secondString, NULL, 16);

}




void interpreter(Bus* bus){
  uint8_t oppCode;
  uint16_t i;
  uint16_t j;
  uint16_t start;
  uint16_t end;
  uint16_t val;
  uint16_t addr;
  char input[MAX_STR]; 
  long inputNum;
  printf("***** type 'h' to print help ******\n");
  while(1){
    oppCode = readBus(bus, bus->cpu->pc); 
    printf("Loaded Instruction: %x \n", oppCode);
    printf("Program Counter: %x \n", bus->cpu->pc);
    printf(">");
    (void)!fgets(input, MAX_STR, stdin);

    if(input[0] == 's'){
      // steps through the cpu program
      decodeAndExecute(bus->cpu, bus, oppCode); 
      bus->cpu->pc++;
    } else if(input[0] == 'p'){
      if(input[1] == 'c'){
        
        bus->cpu->pc = strtol(input + 2, NULL, 16);;
        

      } else {

      parseTwoHexNums(input+1, &start, &end);

      // code to format memory contents on screen
      printf("     00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f \n");
      i = start;
      // TODO: figure out what i wrote here :,)
      for(j = i & 0xfff0; j < i; j++){
        if(j == (i & 0xfff0)){
          printf("%.4x ", j);
        }
        printf("   ");
      }
      for(i = start; i <= end; ++i){
        if(i % 16 == 0){
          // i mod 16 will print the offset before it prints the contents
          // of that address from 0x0-0xf
          printf("%.4x ", i);
        }
        // prints the contents of whichever address i is, 
        printf("%.2x ", readBus(bus, i));

        // after printing the contents at 0xf offset, it will start a new line
        if(i % 16 == 15){
          printf("\n");
        }
        if(i == 0xffff){
          break;
        }

      }
      printf("\n");
      }
    } else if(input[0] == 'q' || input[0] == 'e'){
      free(bus->cpu);
      free(bus->memArr[0].contents);
      free(bus->memArr[1].contents);
      free(bus->memArr);
      exit(1);
    } else if(input[0] == 'h'){
      printf("Commands: \n");
      printf("\t h - print help \n");
      printf("\t s - step through program \n");
      printf("\t q or e - quit \n");
      printf("\t p[x],[y] - print memory contents \n");
      printf("\t w[val],[addr] - write to memory address \n");
      printf("\t r - print cpu registers \n");
      printf("\t pc[x] - change program counter \n");


    } else if(input[0] == 'w'){

      inputNum = strtol(input + 1, NULL, 16);
      parseTwoHexNums(input + 1, &val, &addr);

      writeBus(bus, addr, (uint8_t)val);

    } else if(input[0] == 'r'){
      
      printf("x: %x \n", bus->cpu->x);
      printf("y: %x \n", bus->cpu->y);
      printf("a: %x \n", bus->cpu->a);
      printf("sp: %x \n", bus->cpu->sp);
      printf("pf: %x \n", bus->cpu->pf);
      printf("pc: %x \n", bus->cpu->pc);
      
    } else {

      printf("Bad Command \n");
    }
    
  } 




}

void startNes(char* romPath, int screenScaling){
  printf("Starting NES emulator \n");

  FILE* romPtr; 
  char byte;
  Bus bus;
  int wrongFileFlag = 0;
  int numOfPrgRoms;
  int numOfChrRoms;
  int cycles;
  uint8_t byte10;
  uint16_t prgNvramSize;

  uint16_t prgRamSize; // 0 - no PRG-RAM
  int tvSystem;
  uint8_t oppCode;
  int scanlines = 0;
  uint8_t tempInt;
  int mirroring;

  if(screenScaling < 1){
    screenScaling = 1;
  }

  SDL_Window* win = SDL_CreateWindow("erNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH * screenScaling, WINDOW_HEIGHT * screenScaling, 0);
  SDL_Renderer *renderer;
  SDL_Texture *texture;

  renderer = SDL_CreateRenderer(win, 1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
  romPtr = fopen(romPath, "r");


  if(romPtr == NULL){
    printf("File not found \n");
    exit(1);
  }

  // parses header and checks to see if it is an .ines file
  // bytes 0-3 of ines header
  for(int i = 0; i < 4; ++i){
    byte = fgetc(romPtr);
    if(i == 0 && byte != 0x4e){
      wrongFileFlag = 1;
    } else if(i == 1 && byte != 0x45){
      wrongFileFlag = 1;
    } else if(i == 2 && byte != 0x53){
      wrongFileFlag = 1;
    } else if(i == 3 && byte != 0x1a){
      wrongFileFlag = 1;
    } 

  }

  if(wrongFileFlag == 1)
    printf("Selected file is not an NES rom \n");


  // byte 4
  numOfPrgRoms = fgetc(romPtr);

  // byte 5
  numOfChrRoms = fgetc(romPtr);

  // byte 6
  tempInt = fgetc(romPtr);
  bus.mapper = (tempInt >> 4) & 0b1111;

 
  mirroring = getBit(tempInt, 0);

  // byte 7
  bus.mapper = (bus.mapper | ((fgetc(romPtr) & 0b11110000)));
  
  // byte 8
  fgetc(romPtr);

  // byte 9
  fgetc(romPtr);
  // byte 10
  byte10 = fgetc(romPtr);

  // first check for PRG-RAM
  prgRamSize = 64 << (byte10 & 0b1111);
  if(prgRamSize == 64){
    prgRamSize = 0;
  }


  // second check for PRG-NVRAM (PRG-RAM and PRG-NVRAM are mutually exclusive)
  if(prgRamSize == 0){
    prgRamSize = 64 << ((byte10 & 0b11110000) >> 4);
    if(prgRamSize == 64){
      prgRamSize = 0;
    }
  }

  // byte 11
  fgetc(romPtr);
  // byte 12
  tvSystem = fgetc(romPtr);
  if(tvSystem == 1 || tvSystem == 3){
    printf("Error: PAL Rom detected \n");
    freeAndExit(&bus); 
  }
  
  // next 7 bytes are not needed (see ines header format spec)
  for(int i = 0; i < 3; ++i){
    fgetc(romPtr);
  }
  


  switch(bus.mapper){

    case 0:
      printf("mapper 0 \n");
      // NROM Mapper
      // Setup NROM mapper and starts the main loop

      // sets up address space and dumps rom contents into memory
      initBus(&bus, numOfPrgRoms + 1);
      initMemStruct(&(bus.memArr[0]), 0x0800, Ram, TRUE);
      initMemStruct(&(bus.memArr[1]), 0x8000, Rom, TRUE);


      // + 2 because we have CHR-ROM/RAM plus the two nametables we have to allocate.
      initPpu(bus.ppu, numOfChrRoms + 2);
      printf("%d \n", numOfChrRoms + 2);
      populatePalette(bus.ppu);
      
      

      // loads prg-rom into memory
      if(numOfPrgRoms == 1){
        for(int i = 0; i < (16384 * numOfPrgRoms); ++i){
          bus.memArr[1].contents[i + 0x4000] = fgetc(romPtr);
        }
      }else if(numOfPrgRoms == 2){
       for(int i = 0; i < (16384 * numOfPrgRoms); ++i){
          bus.memArr[1].contents[i] = fgetc(romPtr);
        }
      }
      if(numOfChrRoms == 0){

        initMemStruct(&(bus.ppu->ppubus->memArr[0]), 0x2000, Ram, TRUE);
      } else {
        initMemStruct(&(bus.ppu->ppubus->memArr[0]), 0x2000, Rom, TRUE);
      }
      initMemStruct(&(bus.ppu->ppubus->memArr[1]), 0x2000, Rom, TRUE);
      initMemStruct(&(bus.ppu->ppubus->memArr[2]), 0x2000, Rom, TRUE);
      // loads chr-rom into ppu memory
      for(int i = 0; i < (8192 * numOfChrRoms); ++i){
        bus.ppu->ppubus->memArr[0].contents[i] = fgetc(romPtr);

      }
      
      reset(bus.cpu, &bus);
      resetPpu(bus.ppu, 1);

      bus.ppu->mapper = bus.mapper;
      if(numOfChrRoms == 0){
        bus.ppu->flagChrRam = 1;
      } else {
        bus.ppu->flagChrRam = 0;
      }

      bus.ppu->mirroring = mirroring;

      if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
      }

      SDL_RenderClear(renderer);
      SDL_RenderPresent(renderer);
      printf("SDL initialized! \n");
 
      nesMainLoop(&bus, renderer, texture, screenScaling);
      break;
    

    case 1: 
      printf("mapper 1 \n");
      printf("prgramsize %x \n", prgRamSize);

      printf("numofprgrom: %d \n", numOfPrgRoms);
      printf("numofchrrrom %d \n", numOfChrRoms);
      if(prgRamSize == 0){
        printf("prg ram not present \n");
        // + 1 because we have to account for 0x000-0x7ff ram, alongside the PRG-ROM
        initBus(&bus, numOfPrgRoms + 1);
        bus.presenceOfPrgRam = 0;
      
      } else {
        // + 2 because we have to account for 0x000-0x7ff ram, along with the PRG-RAM and PRG-ROM
        initBus(&bus, numOfPrgRoms + 2);
        bus.presenceOfPrgRam = 1;
      }
      // this is initializing 0x0000-0x07ff RAM
      initMemStruct(&(bus.memArr[0]), 0x0800, Ram, TRUE);

      if(prgRamSize == 0){
        for(int i = 1; i < numOfPrgRoms + 1; ++i){
          initMemStruct(bus.memArr + i, 0x4000, Rom, TRUE);
        }
        
      } else {
        // PRG-RAM gets allocated first ($6000-$7fff)
        
          initMemStruct(bus.memArr + 1, prgRamSize, Ram, TRUE);
          /*
          for(int i = 0; i < prgRamSize; ++i){
            bus.memArr[1].contents[i] = rand() % (255 - 1);
          }
          */
       
        
        for(int i = 2; i < numOfPrgRoms + 2; ++i){
          initMemStruct(bus.memArr + i, 0x4000, Rom, TRUE);
        }

      }
      
      if(numOfChrRoms > 0){
        // this is (numOfChrroms * 2) + 2 because numofchrroms * 2 equals the amount of 4KB bank pattern tables we need to allocate and + 2 because
        // we need to allocate the two nametables.
        initPpu(bus.ppu, (numOfChrRoms * 2) + 2);
      } else if(numOfChrRoms == 0){
        // 4 because we need to allocate memory for both pattern tables and the two nametables
        initPpu(bus.ppu, 4);
      }
      populatePalette(bus.ppu);

      if(numOfChrRoms == 0){
        initMemStruct(&(bus.ppu->ppubus->memArr[0]), 0x1000, Ram, TRUE);
        initMemStruct(&(bus.ppu->ppubus->memArr[1]), 0x1000, Ram, TRUE);
        initMemStruct(&(bus.ppu->ppubus->memArr[2]), 0x400, Ram, TRUE);
        initMemStruct(&(bus.ppu->ppubus->memArr[3]), 0x400, Ram, TRUE);
        
      } else {
        printf("setting up chrroms \n");
        for(int i = 0; i < numOfChrRoms * 2; ++i){
          initMemStruct(&(bus.ppu->ppubus->memArr[i]), 0x1000, Rom, TRUE);
        }
        initMemStruct(&(bus.ppu->ppubus->memArr[numOfChrRoms * 2]), 0x400, Ram, TRUE);
        initMemStruct(&(bus.ppu->ppubus->memArr[(numOfChrRoms * 2) + 1]), 0x400, Ram, TRUE);

      }
      
      int offset;
      prgRamSize == 0 ? (offset = 1) : (offset = 2);
      for(int i = 0; i < numOfPrgRoms; ++i){
        for(int j = 0; j < 0x4000; ++j){
          bus.memArr[i + offset].contents[j] = fgetc(romPtr);
        }
      }

      // reserves chunks of 4KB of memory, so that they can be banked in and out by the MMC1 mapper
      for(int i = 0; i < (numOfChrRoms * 2); ++i){
        for(int j = 0; j < 0x1000; ++j){
          bus.ppu->ppubus->memArr[i].contents[j] = fgetc(romPtr);
        }

      }
      reset(bus.cpu, &bus);
      resetPpu(bus.ppu, 1);
      bus.ppu->mapper = bus.mapper;



      if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
      }

      SDL_RenderClear(renderer);
      SDL_RenderPresent(renderer);
      printf("SDL initialized! \n");
  
      bus.ppu->mirroring = mirroring;

      // once machine has been setup to the mapper's needs, enter main loop
      nesMainLoop(&bus, renderer, texture, screenScaling);

      break;
    case 2:
      printf("mapper 2 \n");
      initBus(&bus, numOfPrgRoms + 1);
      printf("numofprgroms %x \n", bus.numOfBlocks);
      initMemStruct(&(bus.memArr[0]), 0x0800, Ram, TRUE);
      for(int i = 1; i < numOfPrgRoms + 1; ++i){
        initMemStruct(bus.memArr + i, 0x4000, Rom, TRUE);
      }
      
      if(numOfChrRoms == 0){
        initPpu(bus.ppu, numOfChrRoms + 3);
      } else {
        initPpu(bus.ppu, numOfChrRoms + 2);
      } 
      populatePalette(bus.ppu);
 
      // zero chr-roms signifies there is one CHR-RAM connected
      if(numOfChrRoms == 0){
        initMemStruct(&(bus.ppu->ppubus->memArr[0]), 0x2000, Ram, TRUE);
      } else {
        initMemStruct(&(bus.ppu->ppubus->memArr[0]), 0x2000, Rom, TRUE);
      }

      // allocate two nametables
      initMemStruct(&(bus.ppu->ppubus->memArr[1]), 0x400, Ram, TRUE);
      initMemStruct(&(bus.ppu->ppubus->memArr[2]), 0x400, Ram, TRUE);
      
      for(int i = 0; i < numOfPrgRoms; ++i){
        for(int j = 0; j < 0x4000; ++j){
          bus.memArr[i + 1].contents[j] = fgetc(romPtr);
        }
      }
      for(int i = 0; i < 0x2000; ++i){
        bus.ppu->ppubus->memArr[0].contents[i] = fgetc(romPtr);

      }
      reset(bus.cpu, &bus);
      resetPpu(bus.ppu, 1);
      bus.ppu->mapper = bus.mapper;



      if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
      }

      SDL_RenderClear(renderer);
      SDL_RenderPresent(renderer);
      printf("SDL initialized! \n");
  
      bus.ppu->mirroring = mirroring;

      // once machine has been setup to the mapper's needs, enter main loop
      nesMainLoop(&bus, renderer, texture, screenScaling);

      break;
    case 3:
      printf("mapper 3 \n");
      printf("nnumofprgroms: %d \n", numOfChrRoms);
      initBus(&bus, numOfPrgRoms + 1);
      initMemStruct(&(bus.memArr[0]), 0x0800, Ram, TRUE);
      for(int i = 1; i < numOfPrgRoms + 1; ++i){
        initMemStruct(bus.memArr + i, 0x4000, Rom, TRUE);
      }

      initPpu(bus.ppu, numOfChrRoms + 2);
      populatePalette(bus.ppu);
      for(int i = 0; i < numOfChrRoms; ++i){
        initMemStruct(&(bus.ppu->ppubus->memArr[i]), 0x2000, Rom, TRUE);
      }
      // allocate the two nametables at the end
      initMemStruct(&(bus.ppu->ppubus->memArr[numOfChrRoms]), 0x400, Ram, TRUE);
      initMemStruct(&(bus.ppu->ppubus->memArr[numOfChrRoms + 1]), 0x400, Ram, TRUE);
      for(int i = 0; i < numOfPrgRoms; ++i){
        for(int j = 0; j < 0x4000; ++j){
          bus.memArr[i + 1].contents[j] = fgetc(romPtr);
        }
      }
 
      for(int i = 0; i < numOfChrRoms; ++i){
        for(int j = 0; j < 0x2000; ++j){
          bus.ppu->ppubus->memArr[i].contents[j] = fgetc(romPtr);
        }
      }

      reset(bus.cpu, &bus);
      resetPpu(bus.ppu, 1);
      bus.ppu->mapper = bus.mapper;



      if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
      }

      SDL_RenderClear(renderer);
      SDL_RenderPresent(renderer);
      printf("SDL initialized! \n");
  
      bus.ppu->mirroring = mirroring;

      // once machine has been setup to the mapper's needs, enter main loop
      nesMainLoop(&bus, renderer, texture, screenScaling);

      break;
    case 7:
      printf("numofprgroms %x \n", numOfPrgRoms);
      printf("numofchrroms %d \n", numOfChrRoms);
      initBus(&bus, numOfPrgRoms + 1);
      initMemStruct(&(bus.memArr[0]), 0x0800, Ram, TRUE);


      for(int i = 1; i < (numOfPrgRoms / 2) + 1; ++i){
        initMemStruct(&(bus.memArr[i]), 0x8000, Rom, TRUE);
      }
      initPpu(bus.ppu, 3);
      initMemStruct(&(bus.ppu->ppubus->memArr[0]), 0x2000, Ram, TRUE);
      initMemStruct(&(bus.ppu->ppubus->memArr[1]), 0x400, Ram, TRUE);
      initMemStruct(&(bus.ppu->ppubus->memArr[2]), 0x400, Ram, TRUE);
      populatePalette(bus.ppu);

      for(int j = 0; j < numOfPrgRoms / 2; ++j){
        for(int i = 0; i < 2; ++i){
          for(int k = 0; k < 0x4000; ++k){
            if(i == 0){
              bus.memArr[j + 1].contents[k] = fgetc(romPtr);
            } else {
              bus.memArr[j + 1].contents[k + 0x4000] = fgetc(romPtr);
            }
          }
        }
      }
      reset(bus.cpu, &bus);
      resetPpu(bus.ppu, 1);
      bus.ppu->mapper = bus.mapper;

      if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
      }

      SDL_RenderClear(renderer);
      SDL_RenderPresent(renderer);
      printf("SDL initialized! \n");
  
      bus.ppu->mirroring = mirroring;
      nesMainLoop(&bus, renderer, texture, screenScaling);

      break;
    default:
      printf("mapper is not compatible \nincompatible rom \n");
      exit(0);
      break;

  }


  
    SDL_Quit();
    freeAndExit(&bus);
    
    return;
  


}

// nesMainLoop()
//   decodes and executes 1 scanline worth of instructions, then instructs ppu to render the scanline
//   once a 240 scanlines have been rendered, draw framebuffer to SDL and enable a vblank
void nesMainLoop(Bus* bus, SDL_Renderer* renderer, SDL_Texture* texture, int screenScaling){
      uint8_t oppCode;
      int mirroring = bus->ppu->mirroring;
      SDL_Event event;
      uint64_t freq = SDL_GetPerformanceFrequency();
      uint64_t frame_start = 0;
      uint64_t frame_end = 0;
      double elasped_ms;
      int sdlFrames = 0;
      int fps_lastTime = SDL_GetTicks();
      int fps_current = 0;
      int processLightGunInput = 0;
      const double target_fps = 60.0;
      const double target_frame_time = 1000.0 / target_fps;
      int mouseX;
      int mouseY;



      // enter main loop
      while(1){
      // mark time at the start of the frame being drawn
 
        if(bus->cpu->cycles < CPU_CYCLES_PER_SCANLINE){
          oppCode = readBus(bus, bus->cpu->pc);
          bus->cpu->cycles += decodeAndExecute(bus->cpu, bus, oppCode);

          //printf("cycles total: %d \n", bus.cpu->cycles);
        } else if(bus->cpu->cycles >= CPU_CYCLES_PER_SCANLINE){
          // render a scanline except while in vblank and during the prerender scanline (261)
          if(bus->ppu->vblank == 0 && bus->ppu->prerenderScanlineFlag == 0){
            renderScanline(bus->ppu);
          }


          if(bus->ppu->scanLine == 0){
            frame_start = SDL_GetPerformanceCounter();

          }
            
            if(bus->ppu->scanLine == 240){
   
              vblankStart(bus);
              drawFrameBuffer(bus->ppu, renderer, texture);
 
              
              //printNameTable(bus);
            } else if(bus->ppu->scanLine == 260){
              vblankEnd(bus);

            } else if(bus->ppu->scanLine == 261){
              prerenderScanline(bus);
              
              // after prerenderscanline, mark the end of the frame, then delay until the next frame is drawn
              frame_end = SDL_GetPerformanceCounter();
              elasped_ms = (frame_end - frame_start) * 1000.0 / freq;
              if(elasped_ms < target_frame_time){
                SDL_Delay((uint32_t)(target_frame_time - elasped_ms));
              }

              sdlFrames++;
              if(fps_lastTime < SDL_GetTicks() - 1000){
                fps_lastTime = SDL_GetTicks();
                fps_current = sdlFrames;
                sdlFrames = 0;
                if(fps_current != 1){
                  printf("fps: %d \n", fps_current);
                }

              }
            if(processLightGunInput >= 1 && processLightGunInput <= 2){
              printf("frame processed %d \n", bus->ppu->frames);
              printf("%x \n", bus->ppu->frameBuffer[mouseY / screenScaling][mouseX / screenScaling]);
              if(bus->ppu->frameBuffer[mouseY / screenScaling][mouseX / screenScaling] == 0xffffff || bus->ppu->frameBuffer[mouseY / screenScaling][mouseX / screenScaling] == 0xffc6c3){
                bus->controller2.lightSensor = 0;
                processLightGunInput = 0;
                printf("detected! \n");
    
              } else {
                bus->controller2.lightSensor = 1;
                processLightGunInput++;
              }
            }

          // polls for events at the end of each prerender scanline (once per frame)
          while (SDL_PollEvent(&event)) {
              switch (event.type) {
                case SDL_QUIT:
                  SDL_Quit(); 
                  freeAndExit(bus);
                  break;
              
                case SDL_KEYDOWN:
                  switch(event.key.keysym.sym){
                    case SDLK_x:
                      bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 0);
                      break;
                    case SDLK_z:
                      bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 1);
                      break;
                    case SDLK_RSHIFT:
                      bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 2);
                      break;
                    case SDLK_RETURN:
                      bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 3);
                      break;
                    case SDLK_UP:
                      bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 4);
                      break;
                    case SDLK_DOWN:
                      bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 5);
                      break;
                    case SDLK_LEFT:
                      bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 6);
                      break;
                    case SDLK_RIGHT:
                      bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 7);
                      break;

                  }
                  break;
                case SDL_KEYUP:
                  switch(event.key.keysym.sym){
                    case SDLK_x:
                      bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 0);
                      break;
                    case SDLK_z:
                      bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 1);
                      break;
                    case SDLK_RSHIFT:
                      bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 2);
                      break;
                    case SDLK_RETURN:
                      bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 3);
                      break;
                    case SDLK_UP:
                      bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 4);
                      break;
                    case SDLK_DOWN:
                      bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 5);
                      break;
                    case SDLK_LEFT:
                      bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 6);
                      break;                  

                    case SDLK_RIGHT:
                      bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 7);
                      break;
                  }
                  break;
                  case SDL_MOUSEBUTTONDOWN:
                    processLightGunInput = 1;
                    mouseX = event.motion.x;
                    mouseY = event.motion.y;
                    printf("frame received %d \n", bus->ppu->frames);
                    bus->controller2.triggerPulled = 1;
                    break;
                    
                  case SDL_MOUSEBUTTONUP:
                    bus->controller2.triggerPulled = 0;
                    break;
                }
              }


            }

            bus->ppu->scanLine++;
            bus->ppu->scanLineSprites++;
            bus->ppu->frames++;

            bus->cpu->cycles = 0;

            if(bus->ppu->scanLine == 262){
              bus->ppu->scanLine = 0;
              bus->ppu->scanLineSprites = -1;
            }

        }
        

 
       
  }
}


void dumpFileToMemory(uint8_t* fileBuffer, Mem* mem, int offset, int size){
  
  if(offset + size > mem->size)
    return;

  for(int i = offset; i < size; ++i){
    mem->contents[i] = fileBuffer[i];
  }

}




// helper function to populate the processorState struct
// can send a
void populateProcStructWithJson(cJSON* json, processorState* procStat, uint8_t opcode){

  procStat->x = cJSON_GetObjectItemCaseSensitive(json, "x")->valueint;
  procStat->y = cJSON_GetObjectItemCaseSensitive(json, "y")->valueint;
  procStat->a = cJSON_GetObjectItemCaseSensitive(json, "a")->valueint;
  procStat->p = cJSON_GetObjectItemCaseSensitive(json, "p")->valueint;
  procStat->s = cJSON_GetObjectItemCaseSensitive(json, "s")->valueint;
  procStat->pc = cJSON_GetObjectItemCaseSensitive(json, "pc")->valueint;
  char tempStr[MAX_STR];
  procStat->lastOpCode = opcode;



}


void printCpu(CPU* cpu){
  fputs("CPU: \n", stdout);
  printf("x: %.4d \n", cpu->x);
  printf("y: %.4d \n", cpu->y);
  printf("a: %.4d \n", cpu->a);
  printf("pf: %.4d \n", cpu->pf);
  printf("sp: %.4d \n", cpu->sp);
  printf("pc: %.4d \n", cpu->pc);

}



// prints the contents of the cpu and if the test failed it will print what the final value was supposed to be
void printCpuWithJson(CPU* cpu, processorState final, int errorCode){
  fputs("CPU:       JSON:\n", stdout);
  printf("x: %.4d  - %.4d \n", cpu->x, final.x);
  printf("y: %.4d  - %.4d \n", cpu->y, final.y);
  printf("a: %.4d  - %.4d \n", cpu->a, final.a);
  printf("pf: %.4d  - %.4d \n", cpu->pf, final.p);
  printf("sp: %.4d - %.4d \n", cpu->sp, final.s);
  printf("pc: %.4d - %.4d \n", cpu->pc, final.pc);

}

void printProcState(processorState* state){
  fputs("State: \n", stdout);
  printf("x: %.4d \n", state->x);
  printf("y: %.4d \n", state->y);
  printf("a: %.4d \n", state->a);
  printf("pf: %.4d \n", state->p);
  printf("sp: %.4d \n", state->s);
  printf("pc: %.4d \n", state->pc);

}

// checks to see if the processor state matches the json data
// returns an error code:
// bitPos - cpu register
// 0 - x
// 1 - y
// 2 - a
// 3 - pf
// 4 - sp
// 5 - pc
//
int checkProcState(CPU* cpu, cJSON* json){


  int errorCode = 0;
  if(cpu->x != cJSON_GetObjectItemCaseSensitive(json, "x")->valueint){
    errorCode = setBit(errorCode, 0);
  } 

  if(cpu->y != cJSON_GetObjectItemCaseSensitive(json, "y")->valueint){
   errorCode = setBit(errorCode, 1);
  } 

  if(cpu->a != cJSON_GetObjectItemCaseSensitive(json, "a")->valueint){
    errorCode = setBit(errorCode, 2);
  } 

  if(cpu->pf != cJSON_GetObjectItemCaseSensitive(json, "p")->valueint){
    errorCode = setBit(errorCode, 3);
  } 

  if(cpu->sp != cJSON_GetObjectItemCaseSensitive(json, "s")->valueint){
    errorCode = setBit(errorCode, 4);
  } 

  if(cpu->pc != cJSON_GetObjectItemCaseSensitive(json, "pc")->valueint){
    errorCode = setBit(errorCode, 5);
  }
  return errorCode;


}


int populateProcStateWithJson(cJSON* json, Bus* bus){


  bus->cpu->x = cJSON_GetObjectItemCaseSensitive(json, "x")->valueint;
  bus->cpu->y = cJSON_GetObjectItemCaseSensitive(json, "y")->valueint;
  bus->cpu->a = cJSON_GetObjectItemCaseSensitive(json, "a")->valueint;
  bus->cpu->pf = cJSON_GetObjectItemCaseSensitive(json, "p")->valueint;
  bus->cpu->sp = cJSON_GetObjectItemCaseSensitive(json, "s")->valueint;
  bus->cpu->pc = cJSON_GetObjectItemCaseSensitive(json, "pc")->valueint;


  return 1;
}



// checks memory against a cjson array
// assumes that the json object is called "ram" and it is a 2d array, with every array element having 2 entries
int checkMemWithJson(Bus* bus, cJSON* json){

  uint16_t addr;
  uint8_t val;
  int isFailed = 0;

  for(int i = 0; i < cJSON_GetArraySize(json); ++i){
    addr = cJSON_GetArrayItem(cJSON_GetArrayItem(json,i),0)->valueint;
    val = cJSON_GetArrayItem(cJSON_GetArrayItem(json,i),1)->valueint;
    if(readBus(bus, addr) == val){
      continue;
    } else {
      printf("Incorrect memory value at %d with %d \n", addr, val);
      printf("\tReadbus: %d \n", readBus(bus, addr));
      isFailed = 1;
    }
  }

  return isFailed;

}

void copyProcessorState(processorState* dest, processorState* src){
  dest->a = src->a;
  dest->x = src->x;
  dest->y = src->y;
  dest->pc = src->pc;
  dest->p = src->p;
  dest->s = src->s;
  dest->lastOpCode = src->lastOpCode;
  dest->errorFlag = src->errorFlag;

}

int populateMemWithJson(Bus* bus, cJSON* json){
  uint16_t addr;
  uint8_t val;
  //printf("writing %d values \n", cJSON_GetArraySize(json));

  for(int i = 0; i < cJSON_GetArraySize(json); ++i){
    addr = cJSON_GetArrayItem(cJSON_GetArrayItem(json,i),0)->valueint;
    val = cJSON_GetArrayItem(cJSON_GetArrayItem(json,i),1)->valueint;
    writeBus(bus, addr, val);

    //printf("Writing to bus at %d with %d \n", addr, val);
    //printf("Reading to bus at %d with %d \n", addr, readBus(bus, addr));
    

  }
  return 1;

}

// void jsonTester()
// takes the file directory where the tom harte tests reside and will run them
// ranges from 00.json to ff.json
// inputs:
//   char* - complete path of json file, relative to executable
//   Bus* -  bus to be tested
int jsonTester(char* file, Bus* bus, processorState* state){
  FILE *fptr;


  fptr = fopen(file, "r");
  int fileLength;

  if(fptr == NULL){
    printf("File does not exist \n");
    return -1;
  }

  // determines length of file
  fseek(fptr, 0L, SEEK_END);
  fileLength = ftell(fptr);
  fseek(fptr, 0L, SEEK_SET);


  char fileContents[fileLength+1];
  (void)!fread(fileContents, sizeof(char), fileLength, fptr);
  fileContents[fileLength] = '\0';
  //fgets(fileContents, fileLength, fptr);
  //printf("File Size: %d \n", fileLength);
  //printf("%s \n", fileContents);
  
  
  
  
 
  cJSON* jsonData = cJSON_Parse(fileContents);
  //cJSON* jsonDataArr = cJSON_CreateArray(); 
  //cJSON_AddItemToArray(jsonDataArr, jsonData);
  cJSON* name;
  cJSON* initial;
  cJSON* final;
  cJSON* initRam;
  cJSON* finalRam;
  processorState initStruct;
  processorState finalStruct;

  uint8_t oppCode;
  int errorCode;
  char input[MAX_STR];
  strcpy(input, "0");


  for(int i = 0; i < cJSON_GetArraySize(jsonData); ++i){
  // parse json information from the json array
    name = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(jsonData,i), "name");
    initial = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(jsonData,i), "initial");
    final = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(jsonData,i), "final");
    initRam = cJSON_GetObjectItemCaseSensitive(initial, "ram");
    finalRam = cJSON_GetObjectItemCaseSensitive(final, "ram");
    //pc = cJSON_GetObjectItemCaseSensitive(initial, "pc");
    //cJSON* title = cJSON_GetObjectItemCaseSensitive(glossary, "title");
    //if(SUPPRESSOUTPUT == 0)
     // printf("Test Name %s \n", name->valuestring);
  
    // setup cpu and testing environment

    //printf("memory address of memarr %lx for %s \n", bus->memArr, file);

    reset(bus->cpu, bus);
    clearMem(&bus->memArr[0]);
    populateProcStateWithJson(initial, bus);
    //if(SUPPRESSOUTPUT == 0)
     // printCpu(bus->cpu);
    //populateProcState(final, bus);
    populateMemWithJson(bus, initRam);


    // start cpu and run test
    oppCode = readBus(bus, bus->cpu->pc); 
    //if(SUPPRESSOUTPUT == 0)
    //printf("Executing Oppcode 0x%x at %d\n", oppCode, bus->cpu->pc);
    decodeAndExecute(bus->cpu, bus, oppCode);

    //fputs("\n", stdout);
    populateProcStructWithJson(final, &finalStruct, oppCode);
    //if(SUPPRESSOUTPUT == 0)
    //  printCpuWithJson(bus->cpu, finalStruct, errorCode);


    if(checkProcState(bus->cpu, final) == 0 && checkMemWithJson(bus, finalRam) == 0){
      //if(SUPPRESSOUTPUT == 0)
       // printf("Passed! \n \n");
      
    } else {
        printf("Failed! \n Passed %d tests! \n", i);
        printf("Test Name %s \n", name->valuestring);
        printCpuWithJson(bus->cpu, finalStruct, errorCode);
      if(state != NULL)
        copyProcessorState(state, &finalStruct);
      return 0;
   
    }


  //  while(input[0] != 'c'){
  //    fgets(input, MAX_STR, stdin);

  //  }
  //strcpy(input, "0");
  
  }

   //int val = cJSON_GetArrayItem(cJSON_GetArrayItem(initRam,0),1)->valueint;
//  char* str = cJSON_Print(cJSON_GetArrayItem(cJSON_GetArrayItem(initRam,0),0));
  //printf("%s \n", str);
  //printf("%d \n", val);
   



   if(state != NULL)
        copyProcessorState(state, &finalStruct);
   
  return 1;

}


void printHelp(){

  puts("NES/6502 emulator \n");
  puts("Emulates a 6502 mcu as well as an NES");
  puts("Usage: nesemu [-n][FILE]\n");
  puts("       nesemu [-p] [DIR] \n");
  puts("       nesemu [-i] \n");
  puts("       nesemu [-h] \n");
  puts("       nesemu [-n] [FILE] -s [SCALING INTEGER] \n");
  puts("\t -j [FILE] \t starts json tester with specfic json file \n");
  puts("\t -i [DIR] \t starts interpreter with 64k allocated to RAM \n");
  puts("\t -n [FILE] \t starts in NES mode with INES rom file \n");
    puts("\t -s [RESOLUTION SCALING INTEGER] \t integer amount to scale the resolution by (default: 1) \n");
  puts("\t NOTE: To use -j or -i flags, make sure to set the NESEMU macro in general.h and recompile ");


  exit(0);
}


void freeAndExit(Bus* bus){
  

  printf("Freeing memory and exiting... \n");

  for(int i = 0; i < WINDOW_HEIGHT; ++i){

    free(bus->ppu->frameBuffer[i]);
  }

  free(bus->ppu->frameBuffer);
  free(bus->ppu->chrrom);
  free(bus->ppu->oam);
  free(bus->ppu->paletteram);
  for(int i = 0; i < bus->ppu->ppubus->numOfBlocks; ++i){
    free(bus->ppu->ppubus->memArr[i].contents);
  }
  free(bus->ppu->ppubus->memArr);
  free(bus->ppu->ppubus);
  free(bus->ppu);
  free(bus->cpu);

  
  free(bus->memArr);
  
  exit(0);




}

