#include "stdafx.h"
#include "Board.h"

#include <iostream>

Board::Board(const Configuration& configuration)
: m_configuration(configuration) {
}

void Board::initialise() {

	if (m_configuration.isDebugMode()) {

		CPU().ExecutingInstruction.connect([this] (const EightBit::GameBoy::LR35902&) {
			if (IO().bootRomDisabled()) {
				std::cerr << EightBit::GameBoy::Disassembler::state(CPU());
				m_disassembled = m_disassembler.disassemble(CPU());
			}
		});

		CPU().ExecutedInstruction.connect([this](const EightBit::GameBoy::LR35902&) {
			if (IO().bootRomDisabled()) {
				std::cerr << " CYC:" << CPU().cycles() << "\t" << m_disassembled << std::endl;
			}
		});
	}

	WrittenByte.connect([this] (const EightBit::EventArgs&) {
		switch (ADDRESS().word) {
		case EightBit::GameBoy::IoRegisters::BASE + EightBit::GameBoy::IoRegisters::SB:
			std::cout << DATA();
			break;
		default:
			break;
		}
	});

	const auto romDirectory = m_configuration.getRomDirectory();
	loadBootRom(romDirectory + "/DMG_ROM.bin");
}

void Board::plug(std::string path) {
	const auto romDirectory = m_configuration.getRomDirectory();
	loadGameRom(romDirectory + "/" + path);
}
