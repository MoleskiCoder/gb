#include "stdafx.h"
#include "Board.h"

#include <iostream>

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_profiler(CPU()) {
}

void Board::initialise() {

	if (m_configuration.isProfileMode()) {
		CPU().ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Profile, this, std::placeholders::_1));
	}

	if (m_configuration.isDebugMode()) {
		CPU().ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
	}

	WrittenByte.connect(std::bind(&Board::Bus_WrittenByte, this, std::placeholders::_1));

	const auto romDirectory = m_configuration.getRomDirectory();
	loadBootRom(romDirectory + "/DMG_ROM.bin");
}

void Board::plug(const std::string& path) {
	const auto romDirectory = m_configuration.getRomDirectory();
	loadGameRom(romDirectory + "/" + path);
}

void Board::Cpu_ExecutingInstruction_Profile(const EightBit::GameBoy::LR35902& cpu) {
	const auto pc = CPU().PC();
	m_profiler.add(pc.word, peek(pc.word));
}

void Board::Cpu_ExecutingInstruction_Debug(const EightBit::GameBoy::LR35902& cpu) {
	if (IO().bootRomDisabled())
		std::cerr
			<< EightBit::GameBoy::Disassembler::state(CPU())
			<< " "
			<< m_disassembler.disassemble(CPU())
			<< '\n';
}

void Board::Bus_WrittenByte(const EightBit::EventArgs& e) const {
	switch (ADDRESS().word) {
	case EightBit::GameBoy::IoRegisters::BASE + EightBit::GameBoy::IoRegisters::SB:
		std::cout << DATA();
		break;
	}
}
