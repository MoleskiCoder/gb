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
	EightBit::LR35902& CPU() { return m_cpu; }

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
		BUS().triggerInterrupt(EightBit::Bus::Interrupts::VerticalBlank);
		int cycles = 0;
		for (int line = 0; line < (EightBit::Bus::TotalLineCount - Display::RasterHeight); ++line)
			cycles += runHorizontalLine();
		return cycles;
	}

	int runHorizontalLine() {
		DrawingLine.fire(*this);
		auto cycles = m_cpu.run(getCyclesPerLine());
		BUS().incrementLY();
		if ((BUS().readRegister(EightBit::Bus::STAT) & EightBit::Processor::Bit6) && (BUS().readRegister(EightBit::Bus::LYC) == BUS().readRegister(EightBit::Bus::LY)))
			BUS().triggerInterrupt(EightBit::Bus::Interrupts::DisplayControlStatus);
		return cycles;
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

	void Bus_WrittenByte(const EightBit::AddressEventArgs& e);
};
