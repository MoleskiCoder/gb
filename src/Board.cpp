#include "stdafx.h"
#include "Board.h"

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_memory(0xffff),
  m_cpu(Z80(m_memory, m_ports)),
  m_power(false) {
}

void Board::initialise() {

	m_power = false;

	m_memory.clear();
	auto romDirectory = m_configuration.getRomDirectory();

	switch (m_configuration.getMachineMode()) {
	case Configuration::ZX81:
		m_memory.loadRom(romDirectory + "/zx81_v2.rom", 0x00);
		m_ports.WritingPort.connect(std::bind(&Board::Board_PortWriting_ZX81, this, std::placeholders::_1));
		m_ports.WrittenPort.connect(std::bind(&Board::Board_PortWritten_ZX81, this, std::placeholders::_1));
		m_ports.ReadingPort.connect(std::bind(&Board::Board_PortReading_ZX81, this, std::placeholders::_1));
		break;

	case Configuration::CPM:
		m_memory.loadRam(romDirectory + "/prelim.com", 0x100);		// Cringle preliminary tests
		//m_memory.loadRam(romDirectory + "/zexdoc.com", 0x100);		// Cringle preliminary tests

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

void Board::Cpu_ExecutingInstruction_Cpm(const Z80&) {
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
	auto c = m_cpu.BC().low;
	switch (c) {
	case 0x2: {
		auto character = m_cpu.DE().low;
		std::cout << character;
		break;
	}
	case 0x9:
		for (uint16_t i = m_cpu.DE().word; m_memory.get(i) != '$'; ++i) {
			std::cout << m_memory.get(i);
		}
		break;
	}
}

void Board::Cpu_ExecutingInstruction_Profile(const Z80& cpu) {

	const auto pc = cpu.getProgramCounter();

	m_profiler.addAddress(pc);
	m_profiler.addInstruction(m_memory.get(pc));
}

void Board::Cpu_ExecutingInstruction_Debug(Z80& cpu) {

	std::cerr
		<< Disassembler::state(cpu)
		<< "\t"
		<< m_disassembler.disassemble(cpu)
		<< '\n';
}

void Board::Board_PortWriting_ZX81(const PortEventArgs& portEvent) {
	auto port = portEvent.getPort();
	auto value = m_ports.readOutputPort(port);
	std::cout << "** Writing to port: " << Disassembler::hex(port) << " Value: " << Disassembler::hex(value) << std::endl;
}

void Board::Board_PortWritten_ZX81(const PortEventArgs& portEvent) {
	auto port = portEvent.getPort();
	auto value = m_ports.readOutputPort(port);
	std::cout << "** Written to port: " << Disassembler::hex(port) << " Value: " << Disassembler::hex(value) << std::endl;
}

void Board::Board_PortReading_ZX81(const PortEventArgs& portEvent) {
	auto port = portEvent.getPort();
	std::cout << "** Reading port: " << Disassembler::hex(port) << std::endl;
}
