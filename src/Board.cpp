#include "stdafx.h"
#include "Board.h"

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_cpu(LR35902(m_bus, m_ports)),
  m_power(false) {
}

void Board::initialise() {

	m_power = false;

	BUS().clear();
	auto romDirectory = m_configuration.getRomDirectory();

	BUS().loadBootRom(romDirectory + "/DMG_ROM.bin");
	BUS().loadRom(romDirectory + "/Tetris (World).gb", 0x0000);

	if (m_configuration.isProfileMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Profile, this, std::placeholders::_1));
	}

	if (m_configuration.isDebugMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
	}

	m_bus.reset();
	m_cpu.initialise();
	m_cpu.setProgramCounter(0);
}

void Board::Cpu_ExecutingInstruction_Profile(const LR35902& cpu) {

	const auto pc = cpu.getProgramCounter();

	m_profiler.addAddress(pc);
	m_profiler.addInstruction(BUS().peek(pc));
}

void Board::Cpu_ExecutingInstruction_Debug(LR35902& cpu) {

	std::cerr
		<< Disassembler::state(cpu)
		<< "\t"
		<< m_disassembler.disassemble(cpu)
		<< '\n';
}
