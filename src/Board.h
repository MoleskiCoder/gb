#pragma once

#include <string>

#include "Bus.h"
#include "InputOutput.h"
#include "Configuration.h"
#include "LR35902.h"
#include "Profiler.h"
#include "Disassembler.h"
#include "EventArgs.h"
#include "Signal.h"

class Board {
public:
	enum {
		BufferWidth = 256,
		BufferHeight = 256,
		RasterWidth = 160,
		RasterHeight = 144,
	};

	Board(const Configuration& configuration);

	Signal<Board> DrawingLine;

	Bus& BUS() { return m_bus; }
	const LR35902& getCPU() const { return m_cpu; }
	LR35902& getCPUMutable() { return m_cpu; }

	void initialise();

	bool powered() const { return m_power; }
	void powerOn() { m_power = true; }
	void powerOff() { m_power = false; }

	int getCyclesPerLine() const {
		return m_configuration.getCyclesPerFrame() / Bus::TotalLineCount;
	}

	int runRasterLines() {
		BUS().resetLY();
		int cycles = 0;
		for (int line = 0; line < Board::RasterHeight; ++line)
			cycles += runHorizontalLine();
		return cycles;
	}

	int runVerticalBlankLines() {
		int cycles = generateInterrupt(0x40);	// VBLANK
		for (int line = 0; line < (Bus::TotalLineCount - RasterHeight); ++line)
			cycles += runHorizontalLine();
		return cycles;
	}

	int runHorizontalLine() {
		DrawingLine.fire(*this);
		auto cycles = runToLimit(getCyclesPerLine());
		BUS().incrementLY();
		if ((BUS().REG_LYC() == BUS().REG_LY()) && (BUS().REG_STAT() & Processor::Bit6))
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
			BUS().REG_IF() = Processor::Bit0;
			break;
		case 0x48:	// LCDC Status
			BUS().REG_IF() = Processor::Bit1;
			break;
		case 0x50:	// Timer Overflow
			BUS().REG_IF() = Processor::Bit2;
			break;
		case 0x58:	// Serial Transfer
			BUS().REG_IF() = Processor::Bit3;
			break;
		case 0x60:	// Hi-Lo of P10-P13
			BUS().REG_IF() = Processor::Bit4;
			break;
		default:
			assert(false);
			break;
		}

		auto interrupt = m_cpu.IME() && (BUS().REG_IE() & BUS().REG_IF());

		BUS().REG_IF() = 0;

		if (interrupt)
			return m_cpu.interrupt(value);

		return 0;
	}

private:
	const Configuration& m_configuration;
	InputOutput m_ports;
	LR35902 m_cpu;
	Bus m_bus;
	bool m_power;

	Profiler m_profiler;
	Disassembler m_disassembler;

	void Cpu_ExecutingInstruction_Cpm(const LR35902& cpu);

	void Cpu_ExecutingInstruction_Debug(LR35902& cpu);
	void Cpu_ExecutingInstruction_Profile(const LR35902& cpu);

	void bdos();
};
