#pragma once

#include <string>

#include "Memory.h"
#include "InputOutput.h"
#include "Configuration.h"
#include "Z80.h"
#include "Ula.h"
#include "Profiler.h"
#include "Disassembler.h"
#include "EventArgs.h"

class Board {
public:
	enum {
		RasterWidth = 256,
		RasterHeight = 192
	};

	Board(const Configuration& configuration);

	Ula& BUS() { return m_bus; }
	const Z80& getCPU() const { return m_cpu; }
	Z80& getCPUMutable() { return m_cpu; }

	void initialise();

	bool powered() const { return m_power; }
	void powerOn() { m_power = true; }
	void powerOff() { m_power = false; }

	void triggerHorizontalRetraceInterrupt();

	static int getNumberOfScanLines50Hz() {
		auto upperBlanking = 56;
		auto displayArea = 192;
		auto lowerBlanking = 56;
		auto verticalRetrace = 6;
		return upperBlanking + displayArea + lowerBlanking + verticalRetrace;
	}

	static int getNumberOfScanLines60Hz() {
		auto upperBlanking = 32;
		auto displayArea = 192;
		auto lowerBlanking = 32;
		auto verticalRetrace = 6;
		return upperBlanking + displayArea + lowerBlanking + verticalRetrace;
	}

	int getNumberOfScanLines() const {
		if (m_configuration.getCyclesPerFrame() == 60)
			return getNumberOfScanLines60Hz();
		return getNumberOfScanLines50Hz();
	}

	static int getCyclesPerScanLine() {
		// Horizontal Scanline Timings
		// Horizontal Display    128 cycles(32 characters, 256 pixels)
		// Horizontal Blanking    64 cycles(left and right screen border)
		// Horizontal Retrace     15 cycles
		// Total Scanline Time   207 cycles
		//
		// Horizontal retrace rate and duration are fixed.The display procedure might increase
		// or decrease the width of the display area(by respectively adjusting the blanking time)
		// even though larger screens might exceed the visible dimensions of the attached TV set or monitor.

		return 207;
	}

	void runScanLine() {
		runToLimit(getCyclesPerScanLine());
	}

	void runToLimit(int limit) {
		for (int cycles = 0; !finishedCycling(limit, cycles); ++cycles) {
			step();
		}
	}

	bool finishedCycling(int limit, int cycles) const {
		auto exhausted = cycles > limit;
		return exhausted || m_cpu.isHalted();
	}

	void step() {
		m_cpu.step();
		auto bit6 = m_cpu.REFRESH() & Processor::Mask6;
		if (bit6 == 0) {
			m_cpu.interruptMaskable();
		}
	}

	uint16_t getVideoMemoryAddress() const {
		return m_cpu.getWord(DFILE);
	}

private:
	enum {
		DFILE = 0x400c
	};

	const Configuration& m_configuration;
	InputOutput m_ports;
	Z80 m_cpu;
	Ula m_bus;
	bool m_power;

	bool m_fiftyHertzRefresh;
	bool m_cassetteInput;

	Profiler m_profiler;
	Disassembler m_disassembler;

	void Cpu_ExecutingInstruction_Cpm(const Z80& cpu);

	void Cpu_ExecutingInstruction_Debug(Z80& cpu);
	void Cpu_ExecutingInstruction_Profile(const Z80& cpu);

	void Board_PortWriting_ZX81(const PortEventArgs& portEvent);
	void Board_PortWritten_ZX81(const PortEventArgs& portEvent);
	void Board_PortReading_ZX81(const PortEventArgs& portEvent);

	void bdos();
};
