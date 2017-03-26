#pragma once

#include <string>

#include "Memory.h"
#include "InputOutput.h"
#include "Configuration.h"
#include "Z80.h"
#include "Profiler.h"
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

private:
	const Configuration& m_configuration;
	Memory m_memory;
	InputOutput m_ports;
	Z80 m_cpu;
	Profiler m_profiler;

	void Cpu_ExecutingInstruction_Cpm(const CpuEventArgs& cpuEvent);

	void Cpu_ExecutingInstruction_Debug(const CpuEventArgs& cpuEvent);
	void Cpu_ExecutingInstruction_Profile(const CpuEventArgs& cpuEvent);

	void bdos();
};
