#pragma once

#include <cstdint>
#include "Memory.h"

class Board;

class Ula : public Memory {
public:
	enum {
		RasterWidth = 256,
		RasterHeight = 192,
		BytesPerPixel = 8,
		BytesPerLine = RasterWidth / BytesPerPixel,
	};

	enum SignalState {
		High, Low,
	};

	Ula(Board& board);

	virtual uint8_t get(int address);

	uint8_t& LINECNTR();
	SignalState& CAS_OUT();
	bool& VERTICAL_RETRACE();
	bool& NMI();

	void incrementLineCounter();

	void clearPixels() {
		m_pixels.fill(0);
	}

	void restartRasterLine() {
		m_rasterY++;
		m_rasterX = 0;
	}

	void restartRasterScreen() {
		m_rasterY = m_rasterX = 0;
	}

	uint8_t& getPixelGroup(int x, int y) {
		return m_pixels[x + y * BytesPerLine];
	}

private:
	Board& m_board;
	uint8_t m_linecntr;
	SignalState m_cas_out;
	bool m_verticalRetrace;
	bool m_nmi;

	std::array<uint8_t, BytesPerLine * RasterHeight> m_pixels;
	int m_rasterX;
	int m_rasterY;
};
