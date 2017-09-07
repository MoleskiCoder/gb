#pragma once

#include <AbstractColourPalette.h>

struct SDL_PixelFormat;

class ColourPalette : public EightBit::GameBoy::AbstractColourPalette {
public:
	void load(SDL_PixelFormat* hardware);
};