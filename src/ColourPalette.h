#pragma once

#include "AbstractColourPalette.h"

struct SDL_PixelFormat;

class ColourPalette : public EightBit::AbstractColourPalette {
public:
	void load(SDL_PixelFormat* hardware);
};