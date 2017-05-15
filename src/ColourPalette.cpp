#include "stdafx.h"
#include "ColourPalette.h"

#include <SDL.h>

ColourPalette::ColourPalette()
: m_colours(4) {
}

void ColourPalette::load(SDL_PixelFormat* hardware) {
	m_colours[Off] = ::SDL_MapRGBA(hardware, 0x9C, 0xBD, 0x0F, SDL_ALPHA_OPAQUE);
	m_colours[Light] = ::SDL_MapRGBA(hardware, 0x8C, 0xAD, 0x0F, SDL_ALPHA_OPAQUE);
	m_colours[Medium] = ::SDL_MapRGBA(hardware, 0x30, 0x62, 0x30, SDL_ALPHA_OPAQUE);
	m_colours[Dark] = ::SDL_MapRGBA(hardware, 0x0F, 0x38, 0x0F, SDL_ALPHA_OPAQUE);
}