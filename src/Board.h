#pragma once

#include <string>

#include <GameBoyBus.h>
#include <Disassembler.h>

#include "Configuration.h"

class Board final : public EightBit::GameBoy::Bus {
public:
	Board(const Configuration& configuration);
	virtual ~Board() {}

	void plug(std::string path);
	void initialise() final;

private:
	const Configuration& m_configuration;

	EightBit::GameBoy::Disassembler m_disassembler = *this;
	std::string m_disassembled;
};
