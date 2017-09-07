#pragma once

#include <string>

#include <GameBoyBus.h>
#include <LR35902.h>
#include <Profiler.h>
#include <Disassembler.h>

#include "Configuration.h"

class Board : public EightBit::GameBoy::Bus {
public:
	Board(const Configuration& configuration);

	EightBit::GameBoy::LR35902& CPU() { return m_cpu; }

	void initialise();

private:
	const Configuration& m_configuration;
	EightBit::GameBoy::LR35902 m_cpu;

	EightBit::GameBoy::Profiler m_profiler;
	EightBit::GameBoy::Disassembler m_disassembler;

	void Cpu_ExecutingInstruction_Debug(const EightBit::GameBoy::LR35902& cpu);
	void Cpu_ExecutingInstruction_Profile(const EightBit::GameBoy::LR35902& cpu);

	void Bus_WrittenByte(const EightBit::AddressEventArgs& e);
};
