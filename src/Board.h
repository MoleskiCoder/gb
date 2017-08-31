#pragma once

#include <string>

#include "Bus.h"
#include "Configuration.h"
#include "LR35902.h"
#include "Profiler.h"
#include "Disassembler.h"

class Board {
public:
	Board(const Configuration& configuration);

	EightBit::Bus& BUS() { return m_bus; }
	EightBit::LR35902& CPU() { return m_cpu; }

	void initialise();

private:
	const Configuration& m_configuration;
	EightBit::LR35902 m_cpu;
	EightBit::Bus m_bus;

	EightBit::Profiler m_profiler;
	EightBit::Disassembler m_disassembler;

	void Cpu_ExecutingInstruction_Debug(const EightBit::LR35902& cpu);
	void Cpu_ExecutingInstruction_Profile(const EightBit::LR35902& cpu);

	void Bus_WrittenByte(const EightBit::AddressEventArgs& e);
};
