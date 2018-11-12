#pragma once

#include <string>

#include <GameBoyBus.h>
#include <Profiler.h>
#include <Disassembler.h>

#include "Configuration.h"

class Board final : public EightBit::GameBoy::Bus {
public:
	Board(const Configuration& configuration);

	void plug(const std::string& path);

protected:
	virtual void initialise() final;

private:
	const Configuration& m_configuration;

	EightBit::GameBoy::Profiler m_profiler;
	EightBit::GameBoy::Disassembler m_disassembler;

	void Cpu_ExecutingInstruction_Debug(const EightBit::GameBoy::LR35902& cpu);
	void Cpu_ExecutingInstruction_Profile(const EightBit::GameBoy::LR35902& cpu);

	void Bus_WrittenByte(const EightBit::EventArgs& e) const;
};
