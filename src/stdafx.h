#pragma once

#include <stdexcept>
#include <string>
#include <iostream>

#include <SDL.h>

#include <Display.h>
#include <AbstractColourPalette.h>
#include <Bus.h>
#include <LR35902.h>
#include <Profiler.h>
#include <Disassembler.h>

#ifdef _MSC_VER
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#endif
