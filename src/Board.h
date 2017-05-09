#pragma once

#include <string>

#include "Memory.h"
#include "InputOutput.h"
#include "Configuration.h"
#include "Z80.h"
#include "Profiler.h"
#include "Disassembler.h"
#include "EventArgs.h"

class Board {
public:
	enum {
		BufferWidth = 256,
		BufferHeight = 256,
		RasterWidth = 160,
		RasterHeight = 144
	};

	Board(const Configuration& configuration);

	Memory& BUS() { return m_bus; }
	const LR35902& getCPU() const { return m_cpu; }
	LR35902& getCPUMutable() { return m_cpu; }

	void initialise();

	bool powered() const { return m_power; }
	void powerOn() { m_power = true; }
	void powerOff() { m_power = false; }

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
	}

private:
	const Configuration& m_configuration;
	InputOutput m_ports;
	LR35902 m_cpu;
	Memory m_bus;
	bool m_power;

	Profiler m_profiler;
	Disassembler m_disassembler;

	void Cpu_ExecutingInstruction_Cpm(const LR35902& cpu);

	void Cpu_ExecutingInstruction_Debug(LR35902& cpu);
	void Cpu_ExecutingInstruction_Profile(const LR35902& cpu);

	void bdos();
};
