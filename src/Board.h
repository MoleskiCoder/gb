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
	virtual void initialise() final;

private:
	const Configuration& m_configuration;

	EightBit::GameBoy::Profiler m_profiler = { *this, CPU() };
	EightBit::GameBoy::Disassembler m_disassembler = *this;
};
