#include "stdafx.h"
#include "Board.h"

// http://problemkaputt.de/zxdocs.htm

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_bus(*this),
  m_cpu(Z80(m_bus, m_ports)),
  m_power(false),
  m_fiftyHertzRefresh(false),
  m_cassetteInput(false) {
	auto fps = configuration.getFramesPerSecond();
	switch (fps) {
	case 50:
	case 60:
		m_fiftyHertzRefresh = (fps == 50);
		break;
	default:
		throw std::logic_error("Unhandled screen refresh rate.");
	}
}

void Board::initialise() {

	m_power = false;

	BUS().clear();
	auto romDirectory = m_configuration.getRomDirectory();

	switch (m_configuration.getMachineMode()) {
	case Configuration::ZX81:
		BUS().loadRom(romDirectory + "/zx81_v2.rom", 0x00);
		m_ports.WritingPort.connect(std::bind(&Board::Board_PortWriting_ZX81, this, std::placeholders::_1));
		m_ports.WrittenPort.connect(std::bind(&Board::Board_PortWritten_ZX81, this, std::placeholders::_1));
		m_ports.ReadingPort.connect(std::bind(&Board::Board_PortReading_ZX81, this, std::placeholders::_1));
		break;

	case Configuration::CPM:
		BUS().loadRam(romDirectory + "/prelim.com", 0x100);		// Cringle preliminary tests
		//m_memory.loadRam(romDirectory + "/zexdoc.com", 0x100);		// Cringle preliminary tests

		BUS().set(5, 0xc9);	// ret
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
		for (uint16_t i = m_cpu.DE().word; BUS().get(i) != '$'; ++i) {
			std::cout << BUS().get(i);
		}
		break;
	}
}

void Board::Cpu_ExecutingInstruction_Profile(const Z80& cpu) {

	const auto pc = cpu.getProgramCounter();

	m_profiler.addAddress(pc);
	m_profiler.addInstruction(BUS().get(pc));
}

void Board::Cpu_ExecutingInstruction_Debug(Z80& cpu) {

	std::cerr
		<< Disassembler::state(cpu)
		<< "\t"
		<< m_disassembler.disassemble(cpu)
		<< '\n';
}

void Board::triggerHorizontalRetraceInterrupt() {
	BUS().incrementLineCounter();
	if (BUS().NMI()) {
		m_cpu.interruptNonMaskable();
	}
}

void Board::Board_PortWriting_ZX81(const PortEventArgs& portEvent) {
}

void Board::Board_PortWritten_ZX81(const PortEventArgs& portEvent) {

	// Output to Port FFh(or ANY other port)
	// Writing any data to any port terminates the Vertical Retrace
	// period, and restarts the LINECNTR counter.The retrace signal
	// is also output to the cassette(ie.the Cassette Output becomes High).
	BUS().VERTICAL_RETRACE() = false;
	BUS().LINECNTR() = 0;
	BUS().CAS_OUT() = Ula::High;

	auto port = portEvent.getPort();
	auto value = m_ports.readOutputPort(port);

	switch (port) {

	// Writing any data to this port disables the NMI generator.
	case 0xfd:
		BUS().NMI() = false;
		std::cout << "Disable NMI" << std::endl;
		break;

	// Writing any data to this port enables the NMI generator.
	// NMIs(Non maskable interrupts) are used during SLOW mode
	// vertical blanking periods to count the number of drawn
	// blank scanlines.
	case 0xfe:
		BUS().NMI() = true;
		std::cout << "Enable NMI" << std::endl;
		break;

	case 0xff:
		std::cout << "Terminate vertical retrace period" << std::endl;
		break;

	default:
		std::cout << "Writing to port: " << Disassembler::hex(port) << "," << Disassembler::hex(value) << std::endl;
		break;
	}
}

void Board::Board_PortReading_ZX81(const PortEventArgs& portEvent) {

	auto port = portEvent.getPort();

	switch (port) {

	// Input from Port FEh(or any other port with A0 zero)
	// Reading from this port initiates the Vertical Retrace period(and
	// accordingly, Cassette Output becomes Low), and resets the LINECNTR
	// register to zero, LINECNTR remains stopped / zero until user terminates
	// retrace - In the ZX81, all of the above happens only if NMIs are disabled.
	//		Bit		Expl.
	//		0 - 4	Keyboard column bits(0 = Pressed)
	//		5		Not used(1)
	//		6		Display Refresh Rate(0 = 60Hz, 1 = 50Hz)
	//		7		Cassette input(0 = Normal, 1 = Pulse)
	// When reading from the keyboard, one of the upper bits(A8 - A15) of
	// the I / O address must be "0" to select the desired keyboard row(0 - 7).
	// (When using IN A, (nn), the old value of the A register is output as
	// upper address bits and <nn> as lower bits.Otherwise, ie.when using
	// IN r, (C) or INI or IND, the BC register is output to the address bus.)
	//
	// The ZX81 / ZX80 Keyboard Matrix
	// Port_____Line______Bit___0_______1____2____3____4__
	// FEFEh	0 (A8)			SHIFT	Z    X    C    V
	// FDFEh	1 (A9)			A		S    D    F    G
	// FBFEh	2 (A10)			Q		W    E    R    T
	// F7FEh	3 (A11)			1		2    3    4    5
	// EFFEh	4 (A12)			0		9    8    7    6
	// DFFEh	5 (A13)			P		O    I    U    Y
	// BFFEh	6 (A14)			ENTER	L    K    J    H
	// 7FFEh	7 (A15)			SPC		.	 M    N    B
	case 0xfe: {

			if (BUS().NMI()) {
				BUS().CAS_OUT() = Ula::Low;
				BUS().LINECNTR() = 0;
				BUS().VERTICAL_RETRACE() = true;
			}

			// The upper address line is the old acculumulator value
			auto upper = m_cpu.A();

			// until I implement proper keyboard input..
			auto keyboardColumnReleased = 0b11111;

			auto refresh = m_fiftyHertzRefresh ? 1 : 0;
			auto cassettInput = m_cassetteInput ? 1 : 0;

			auto value =
				  (cassettInput << 7)
				| (refresh << 6)
				| (1 << 5)
				| keyboardColumnReleased;

			m_ports.writeInputPort(port, value);
		}
		std::cout << "Reading port FE" << std::endl;
		break;
	}
}
