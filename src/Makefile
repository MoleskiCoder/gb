EXE = gb

CXXFLAGS = `sdl2-config --cflags` -I ../libraries/EightBit/Gaming/inc -I ../libraries/EightBit/LR35902/inc -I ../libraries/EightBit/inc -I ../libraries/Gb_Snd_Emu

LDFLAGS  = `sdl2-config --libs` -lSDL2_mixer -L ../libraries/EightBit/LR35902/src -L ../libraries/EightBit/src -L ../libraries/Gaming/src -L ../libraries/Gb_Snd_Emu -lgaming -llr35902 -leightbit -lgbsndemu

CXXFILES = Board.cpp ColourPalette.cpp Computer.cpp gb.cpp

include ../libraries/EightBit/compile.mk
include ../libraries/EightBit/exe_build.mk
