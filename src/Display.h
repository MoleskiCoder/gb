#pragma once

#include <vector>
#include <cstdint>

#include "Bus.h"
#include "ColourPalette.h"

class Display {
public:
	enum {
		BufferWidth = 256,
		BufferHeight = 256,
		BufferCharacterWidth = BufferWidth / 8,
		BufferCharacterHeight = BufferHeight / 8,
		RasterWidth = 160,
		RasterHeight = 144,
	};

	Display(const ColourPalette& colours, Bus& bus);

	const std::vector<uint32_t>& pixels() const;

	void initialise();
	void render();

private:
	std::vector<uint32_t> m_pixels;
	Bus& m_bus;
	const ColourPalette& m_colours;
};