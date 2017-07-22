#include "stdafx.h"
#include "Board.h"

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_cpu(EightBit::LR35902(m_bus)),
  m_power(false),
  m_profiler(m_cpu) {
}

void Board::initialise() {

	m_power = false;

	BUS().clear();
	auto romDirectory = m_configuration.getRomDirectory();

	BUS().loadBootRom(romDirectory + "/DMG_ROM.bin");

	//BUS().loadRam(romDirectory + "/Tetris (World).gb", 0x0000);

	//BUS().loadRam(romDirectory + "/cpu_instrs.gb", 0x0000);					// Loops + failures

	//BUS().loadRam(romDirectory + "/01-special.gb", 0x0000);				// Passed
	//BUS().loadRam(romDirectory + "/02-interrupts.gb", 0x0000);			// EI Failed #130
	//BUS().loadRam(romDirectory + "/03-op sp,hl.gb", 0x0000);				// Failed #129
	//BUS().loadRam(romDirectory + "/04-op r,imm.gb", 0x0000);				// Failed #129
	//BUS().loadRam(romDirectory + "/05-op rp.gb", 0x0000);					// Passed
	//BUS().loadRam(romDirectory + "/06-ld r,r.gb", 0x0000);				// Passed
	//BUS().loadRam(romDirectory + "/07-jr,jp,call,ret,rst.gb", 0x0000);	// Passed
	//BUS().loadRam(romDirectory + "/08-misc instrs.gb", 0x0000);			// Passed
	//BUS().loadRam(romDirectory + "/09-op r,r.gb", 0x0000);				// Failed #129
	//BUS().loadRam(romDirectory + "/10-bit ops.gb", 0x0000);				// Passed
	//BUS().loadRam(romDirectory + "/11-op a,(hl).gb", 0x0000);				// Failed #129
	BUS().loadRam(romDirectory + "/instr_timing.gb", 0x0000);				// Failed #143
	//BUS().loadRam(romDirectory + "/interrupt_time.gb", 0x0000);			// Failed

	//BUS().loadRam(romDirectory + "/opus5.gb", 0x0000);
	//BUS().loadRam(romDirectory + "/ttt.gb", 0x0000);

	BUS().lock(0, 0x8000);

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
