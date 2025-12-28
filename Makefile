CC=gcc
WCC=x86_64-w64-mingw32-gcc-10-posix
CFLAGS= `sdl2-config --cflags --libs` -lcjson -I. -g  -lm 


all: general.o ppu.o cpu.o memory.o main.o
	$(CC) general.o cpu.o memory.o ppu.o main.o $(CFLAGS) -o nesemu

cpu.o: cpu.c 
	$(CC) $(CFLAGS) -c cpu.c

memory.o: memory.c 
	$(CC) $(CFLAGS) -c memory.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c 

ppu.o: ppu.c
	$(CC) $(CFLAGS) -c ppu.c

general.o: general.c
	$(CC) $(CFLAGS) -c general.c




runwfile:
	

clean:
	rm *.o nesemu


