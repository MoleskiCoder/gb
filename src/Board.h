#pragma once

#include <string>

#include "Bus.h"
#include "Configuration.h"
#include "LR35902.h"
#include "Profiler.h"
#include "Disassembler.h"
#include "Signal.h"
#include "Display.h"

class Board {
public:
	Board(const Configuration& configuration);

	EightBit::Signal<Board> DrawingLine;

	EightBit::Bus& BUS() { return m_bus; }
	const EightBit::LR35902& getCPU() const { return m_cpu; }
	EightBit::LR35902& getCPUMutable() { return m_cpu; }

	void initialise();

	bool powered() const { return m_power; }
	void powerOn() { m_power = true; }
	void powerOff() { m_power = false; }

	int getCyclesPerLine() const {
		return m_configuration.getCyclesPerFrame() / EightBit::Bus::TotalLineCount;
	}

	int runRasterLines() {
		BUS().resetLY();
		int cycles = 0;
		for (int line = 0; line < Display::RasterHeight; ++line)
			cycles += runHorizontalLine();
		return cycles;
	}

	int runVerticalBlankLines() {
		int cycles = generateInterrupt(0x40);	// VBLANK
		for (int line = 0; line < (EightBit::Bus::TotalLineCount - Display::RasterHeight); ++line)
			cycles += runHorizontalLine();
		return cycles;
	}

	int runHorizontalLine() {
		DrawingLine.fire(*this);
		auto cycles = runToLimit(getCyclesPerLine());
		BUS().incrementLY();
		if ((BUS().REG(EightBit::Bus::LYC) == BUS().REG(EightBit::Bus::LY)) && (BUS().REG(EightBit::Bus::STAT) & EightBit::Processor::Bit6))
			cycles += generateInterrupt(0x48);
		return cycles;
	}

	int runToLimit(int limit) {
		int cycles = 0;
		do {
			cycles += m_cpu.step();
		} while (cycles < limit);
		return cycles;
	}

	int generateInterrupt(uint8_t value) {

		switch (value) {
		case 0x40:	// VBLANK
			BUS().REG(EightBit::Bus::IF) = EightBit::Processor::Bit0;
			break;
		case 0x48:	// LCDC Status
			BUS().REG(EightBit::Bus::IF) = EightBit::Processor::Bit1;
			break;
		case 0x50:	// Timer Overflow
			BUS().REG(EightBit::Bus::IF) = EightBit::Processor::Bit2;
			break;
		case 0x58:	// Serial Transfer
			BUS().REG(EightBit::Bus::IF) = EightBit::Processor::Bit3;
			break;
		case 0x60:	// Hi-Lo of P10-P13
			BUS().REG(EightBit::Bus::IF) = EightBit::Processor::Bit4;
			break;
		default:
			assert(false);
			break;
		}

		auto interrupt = m_cpu.IME() && (BUS().REG(EightBit::Bus::IE) & BUS().REG(EightBit::Bus::IF));

		BUS().REG(EightBit::Bus::IF) = 0;

		if (interrupt)
			return m_cpu.interrupt(value);

		return 0;
	}

private:
	const Configuration& m_configuration;
	EightBit::LR35902 m_cpu;
	EightBit::Bus m_bus;
	bool m_power;

	EightBit::Profiler m_profiler;
	EightBit::Disassembler m_disassembler;

	void Cpu_ExecutingInstruction_Debug(const EightBit::LR35902& cpu);
	void Cpu_ExecutingInstruction_Profile(const EightBit::LR35902& cpu);
};
