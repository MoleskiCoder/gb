#include "stdafx.h"
#include "Board.h"

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_cpu(LR35902(m_bus)),
  m_power(false) {
}

void Board::initialise() {

	m_power = false;

	BUS().clear();
	auto romDirectory = m_configuration.getRomDirectory();

	BUS().loadBootRom(romDirectory + "/DMG_ROM.bin");
	BUS().loadRom(romDirectory + "/Tetris (World).gb", 0x0000);

	//BUS().loadRom(romDirectory + "/cpu_instrs.gb", 0x0000);

	//BUS().loadRom(romDirectory + "/01-special.gb", 0x0000);
	//BUS().loadRom(romDirectory + "/02-interrupts.gb", 0x0000);
	//BUS().loadRom(romDirectory + "/03-op sp,hl.gb", 0x0000);
	//BUS().loadRom(romDirectory + "/04-op r,imm.gb", 0x0000);
	//BUS().loadRom(romDirectory + "/05-op rp.gb", 0x0000);
	//BUS().loadRom(romDirectory + "/06-ld r,r.gb", 0x0000);
	//BUS().loadRom(romDirectory + "/07-jr,jp,call,ret,rst.gb", 0x0000);
	//BUS().loadRom(romDirectory + "/08-misc instrs.gb", 0x0000);
	//BUS().loadRom(romDirectory + "/09-op r,r.gb", 0x0000);
	//BUS().loadRom(romDirectory + "/10-bit ops.gb", 0x0000);
	//BUS().loadRom(romDirectory + "/11-op a,(hl).gb", 0x0000);
	//BUS().loadRom(romDirectory + "/instr_timing.gb", 0x0000);
	//BUS().loadRom(romDirectory + "/interrupt_time.gb", 0x0000);

	//BUS().loadRom(romDirectory + "/opus5.gb", 0x0000);
	//BUS().loadRom(romDirectory + "/ttt.gb", 0x0000);

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
