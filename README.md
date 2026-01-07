# nesemu
A Nintendo Entertainment System emulator, written in C.

Only compatible with mapper 0 games at the moment.
Able to boot games such as Balloon Fight and Super Mario Bros.

Features a CPU tester to check the core against the Tom Harte processor routines, with the appropriate JSON data.

Currently a work in progress.


## How to compile on a Linux system
``make`` 


## Libraries Needed (Ubuntu)
``sudo apt install libcjson-dev libcjson1 libsdl2-dev libsdl2-2.0-0``

## How to run
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
![Bomberman](https://i.imgur.com/pR5fjyt.png)

