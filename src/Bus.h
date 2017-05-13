#pragma once

#include "Memory.h"

class Bus : public Memory {
public:

	enum {
		TotalLineCount = 154
	};

	enum {
		VideoRam = 0x8000
	};

	enum {

		BASE = 0xFF00,

		// Port/Mode Registers
		P1 = 0x0,
		SB = 0x1,
		SC = 0x2,
		DIV = 0x4,
		TIMA = 0x5,
		TMA = 0x6,
		TAC = 0x7,

		// Interrupt Flags
		IF = 0xF,
		IE = 0xFF,

		// LCD Display Registers
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

		// Sound Registers
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
	};

	Bus();

	void reset();

	uint8_t& REG_P1() { return reference(BASE + P1); }
	uint8_t& REG_SB() { return reference(BASE + SB); }
	uint8_t& REG_SC() { return reference(BASE + SC); }
	uint8_t& REG_DIV() { return reference(BASE + DIV); }
	uint8_t& REG_TIMA() { return reference(BASE + TIMA); }
	uint8_t& REG_TMA() { return reference(BASE + TMA); }
	uint8_t& REG_TAC() { return reference(BASE + TAC); }

	uint8_t& REG_IF() {	return reference(BASE + IF); }
	uint8_t& REG_IE() { return reference(BASE + IE); }

	uint8_t& REG_LCDC() { return reference(BASE + LCDC); }
	uint8_t& REG_STAT() { return reference(BASE + STAT); }
	uint8_t& REG_SCY() { return reference(BASE + SCY); }
	uint8_t& REG_SCX() { return reference(BASE + SCX); }
	uint8_t& REG_LY() { return reference(BASE + LY); }
	uint8_t& REG_LYC() { return reference(BASE + LYC); }
	uint8_t& REG_DMA() { return reference(BASE + DMA); }
	uint8_t& REG_BGP() { return reference(BASE + BGP); }
	uint8_t& REG_OBP0() { return reference(BASE + OBP0); }
	uint8_t& REG_OBP1() { return reference(BASE + OBP1); }
	uint8_t& REG_WY() { return reference(BASE + WY); }
	uint8_t& REG_WX() { return reference(BASE + WX); }

	uint8_t& REG_NR52() { return reference(BASE + NR52); }

	void incrementLY() {
		REG_LY() = (REG_LY() + 1) % TotalLineCount;
	}

	void resetLY() {
		REG_LY() = 0;
	}
};

