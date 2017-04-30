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
		RasterWidth = 256,
		RasterHeight = 192
	};

	Board(const Configuration& configuration);

	Memory& getMemory() { return m_memory; }
	const Z80& getCPU() const { return m_cpu; }
	Z80& getCPUMutable() { return m_cpu; }

	void initialise();

	bool powered() const { return m_power; }
	void powerOn() { m_power = true; }
	void powerOff() { m_power = false; }

private:
	const Configuration& m_configuration;
	Memory m_memory;
	InputOutput m_ports;
	Z80 m_cpu;
	bool m_power;

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
