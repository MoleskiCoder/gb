#include "stdafx.h"
#include "Board.h"

#include <iostream>

Board::Board(const Configuration& configuration)
: m_configuration(configuration) {
}

void Board::initialise() {

	if (m_configuration.isProfileMode()) {
		CPU().ExecutingInstruction.connect([this] (const EightBit::GameBoy::LR35902& cpu) {
			const auto pc = CPU().PC().word;
			m_profiler.add(pc, peek(pc));
		});
	}

	if (m_configuration.isDebugMode()) {
		CPU().ExecutingInstruction.connect([this] (const EightBit::GameBoy::LR35902&) {
			if (IO().bootRomDisabled())
				std::cerr
					<< EightBit::GameBoy::Disassembler::state(CPU())
					<< " "
					<< m_disassembler.disassemble(CPU())
					<< '\n';
		});
	}

	WrittenByte.connect([this] (const EightBit::EventArgs&) {
		switch (ADDRESS().word) {
		case EightBit::GameBoy::IoRegisters::BASE + EightBit::GameBoy::IoRegisters::SB:
			std::cout << DATA();
			break;
		}
	});

	const auto romDirectory = m_configuration.getRomDirectory();
	loadBootRom(romDirectory + "/DMG_ROM.bin");
}

void Board::plug(const std::string& path) {
	const auto romDirectory = m_configuration.getRomDirectory();
	loadGameRom(romDirectory + "/" + path);
}
