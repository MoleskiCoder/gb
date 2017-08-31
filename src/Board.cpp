#include "stdafx.h"
#include "Board.h"

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_cpu(EightBit::LR35902(m_bus)),
  m_profiler(m_cpu) {
}

void Board::initialise() {

	BUS().clear();
	auto romDirectory = m_configuration.getRomDirectory();

	BUS().loadBootRom(romDirectory + "/DMG_ROM.bin");

	//BUS().loadGameRom(romDirectory + "/Tetris (World).gb");

	//BUS().loadGameRom(romDirectory + "/cpu_instrs.gb");				// Passed
	//BUS().loadGameRom(romDirectory + "/01-special.gb");				// Passed
	//BUS().loadGameRom(romDirectory + "/02-interrupts.gb");			// Passed
	//BUS().loadGameRom(romDirectory + "/03-op sp,hl.gb");				// Passed
	//BUS().loadGameRom(romDirectory + "/04-op r,imm.gb");				// Passed
	//BUS().loadGameRom(romDirectory + "/05-op rp.gb");					// Passed
	//BUS().loadGameRom(romDirectory + "/06-ld r,r.gb");				// Passed
	//BUS().loadGameRom(romDirectory + "/07-jr,jp,call,ret,rst.gb");	// Passed
	//BUS().loadGameRom(romDirectory + "/08-misc instrs.gb");			// Passed
	//BUS().loadGameRom(romDirectory + "/09-op r,r.gb");				// Passed
	//BUS().loadGameRom(romDirectory + "/10-bit ops.gb");				// Passed
	//BUS().loadGameRom(romDirectory + "/11-op a,(hl).gb");				// Passed

	BUS().loadGameRom(romDirectory + "/instr_timing.gb");				// Failed #255
	//BUS().loadGameRom(romDirectory + "/interrupt_time.gb");			// Failed

	//BUS().loadGameRom(romDirectory + "/opus5.gb");
	//BUS().loadGameRom(romDirectory + "/ttt.gb");

	if (m_configuration.isProfileMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Profile, this, std::placeholders::_1));
	}

	if (m_configuration.isDebugMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
	}

	m_bus.WrittenByte.connect(std::bind(&Board::Bus_WrittenByte, this, std::placeholders::_1));

	m_bus.reset();
	m_cpu.initialise();

	EightBit::register16_t start;
	start.word = 0;
	m_cpu.PC() = start;
}

void Board::Cpu_ExecutingInstruction_Profile(const EightBit::LR35902& cpu) {
	const auto pc = m_cpu.PC();
	m_profiler.add(pc.word, BUS().peek(pc.word));
}

void Board::Cpu_ExecutingInstruction_Debug(const EightBit::LR35902& cpu) {
	if (m_bus.bootRomDisabled())
		std::cerr
			<< EightBit::Disassembler::state(m_cpu)
			<< " "
			<< m_disassembler.disassemble(m_cpu)
			<< '\n';
}

void Board::Bus_WrittenByte(const EightBit::AddressEventArgs& e) {
	auto address = e.getAddress();
	switch (address) {
	case EightBit::Bus::BASE + EightBit::Bus::SB:
		std::cout << e.getCell();
		break;
	}
}
