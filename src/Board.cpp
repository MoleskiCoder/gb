#include "stdafx.h"
#include "Board.h"
#include "Disassembler.h"

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_memory(0xffff),
  m_cpu(Z80(m_memory, m_ports)) {
}

void Board::initialise() {

	m_memory.clear();
	auto romDirectory = m_configuration.getRomDirectory();

	switch (m_configuration.getMachineMode()) {
	case Configuration::ZX81:
		m_memory.loadRom(romDirectory + "/zx81_v2.rom", 0x00);
		break;

	case Configuration::CPM:
		//m_memory.loadRom(romDirectory + "/TEST.COM", 0x100);		// Microcosm
		//m_memory.loadRom(romDirectory + "/8080PRE.COM", 0x100);	// Bartholomew preliminary
		m_memory.loadRom(romDirectory + "/8080EX1.COM", 0x100);	// Cringle/Bartholomew
		//m_memory.loadRom(romDirectory + "/CPUTEST.COM", 0x100);	// SuperSoft diagnostics

		m_memory.set(5, 0xc9);	// ret
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Cpm, this, std::placeholders::_1));
		break;

	default:
		throw std::logic_error("Unhandled machine type");
	}

	if (m_configuration.isProfileMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Profile, this, std::placeholders::_1));
	}

	if (m_configuration.isDebugMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
	}

	m_cpu.initialise();
	m_cpu.setProgramCounter(m_configuration.getStartAddress());
}

void Board::Cpu_ExecutingInstruction_Cpm(const CpuEventArgs&) {
	auto pc = m_cpu.getProgramCounter();
	switch (pc) {
	case 0x0:	// CP/M warm start
		m_cpu.halt();
		m_profiler.dump();
		break;
	case 0x5:	// BDOS
		bdos();
		break;
	default:
		break;
	}
}

void Board::bdos() {
	auto c = m_cpu.getBC().low;
	switch (c) {
	case 0x2: {
		auto character = m_cpu.getDE().low;
		std::cout << character;
		break;
	}
	case 0x9:
		for (uint16_t i = m_cpu.getDE().word; m_memory.get(i) != '$'; ++i) {
			std::cout << m_memory.get(i);
		}
		break;
	}
}

void Board::Cpu_ExecutingInstruction_Profile(const CpuEventArgs& cpuEvent) {

	const auto& cpu = cpuEvent.getCpu();
	const auto pc = cpu.getProgramCounter();

	m_profiler.addAddress(pc);
	m_profiler.addInstruction(m_memory.get(pc));
}

void Board::Cpu_ExecutingInstruction_Debug(const CpuEventArgs& cpuEvent) {

	std::cerr
		<< Disassembler::state(cpuEvent.getCpu())
		<< "\t"
		<< Disassembler::disassemble(cpuEvent.getCpu())
		<< '\n';
}
