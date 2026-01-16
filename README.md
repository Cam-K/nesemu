# nesemu
A Nintendo Entertainment System emulator, written in C.


## Description

An NES emulator using SDL2 as it's graphics library that is able to boot games such as Castlevania and Super Mario Bros.

Features a CPU tester to check the core against the Tom Harte processor routines, with the appropriate JSON data.

There is no APU imeplementation.

Currently a work in progress.

### Mapper Support:
- Mapper 0 (NROM)
- Mapper 2 (UxROM)
- Mapper 3 (CNROM)


## How to compile on a Linux system
``git clone https://github.com/Cam-K/nesemu.git``  
``cd nesemu``  
``make`` 


## Libraries Needed (Ubuntu)
``sudo apt install libcjson-dev libcjson1 libsdl2-dev libsdl2-2.0-0``

## Usage
### To print help
``./nesemu -h``

### To run a game
``./nesemu -n [FILE]``


## Controls
| Keyboard      | NES Controller   |
|---------------|------------------|
| Arrow Keys    | Control Pad      |
| Z Key         | B Button         |
| X Key         | A Button         |
| Enter         | Start            |
| Shift         | Select           | 
 



## Screenshots

![Super Mario Bros](https://i.imgur.com/ehnudBK.png)
![Castlevania](https://i.imgur.com/eMoT9Kq.png)

