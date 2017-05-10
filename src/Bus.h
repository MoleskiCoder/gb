#pragma once

#include "Memory.h"

class Bus : public Memory {
public:

	enum {
		TotalLineCount = 154
	};

	enum {

		BASE = 0xFF00,

		P1 = 0x0,
		SB = 0x1,
		SC = 0x2,
		DIV = 0x4,
		TIMA = 0x5,
		TMA = 0x6,
		TAC = 0x7,
		IF = 0xF,

		NR10 = 0x10,
		NR11 = 0x11,
		NR12 = 0x12,
		NR13 = 0x13,
		NR14 = 0x14,

		NR21 = 0x16,
		NR22 = 0x17,
		NR23 = 0x18,
		NR24 = 0x19,

		NR30 = 0x1A,
		NR31 = 0x1B,
		NR32 = 0x1C,
		NR33 = 0x1D,
		NR34 = 0x1E,

		NR41 = 0x20,
		NR42 = 0x21,
		NR43 = 0x22,
		NR44 = 0x23,

		NR50 = 0x24,
		NR51 = 0x25,
		NR52 = 0x26,

		WPRAM_START = 0x30,
		WPRAM_END = 0x3F,

		LCDC = 0x40,
		STAT = 0x41,
		SCY = 0x42,
		SCX = 0x43,
		LY = 0x44,
		LYC = 0x45,

		DMA = 0x46,

		BGP = 0x47,

		OBP0 = 0x48,
		OBP1 = 0x49,

		WY = 0x4A,
		WX = 0x4B,

		IE = 0xFF
	};

	Bus();

	uint8_t& REG_IF() {
		return reference(BASE + IF);
	}

	uint8_t& REG_LY() {
		return reference(BASE + LY);
	}

	uint8_t& REG_LYC() {
		return reference(BASE + LYC);
	}

	uint8_t& REG_IE() {
		return reference(BASE + IE);
	}

	uint8_t& REG_STAT() {
		return reference(BASE + STAT);
	}

	void incrementLY() {
		REG_LY() = (REG_LY() + 1) % TotalLineCount;
	}

	void resetLY() {
		REG_LY() = 0;
	}
};

