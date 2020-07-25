# gb
Gameboy emulator

Plays Tetris, lots of bugs!

Uses the Blargg Gb_Snd_Emu library. Sound is now the best part of this emulator!

# Ubuntu Linux

## Prerequisites

### Build prerequisites

`apt-get install build-essential libsdl2-dev libsdl2-mixer-dev libboost-dev`

### ROMs required

1. Original Gameboy boot ROM: "DMG_ROM.bin"
	1. Place in the "roms" folder
1. Game ROM (e.g. Tetris): "Tetris (World).gb"
	1. Place in the "roms/games" folder

## Obtaining source

`git clone https://github.com/MoleskiCoder/gb.git`
`cd gb`
`git submodule update --init --recursive`

## Building

`make`

## Running

`src/gb`
