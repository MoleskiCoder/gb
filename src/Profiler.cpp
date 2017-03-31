#include "stdafx.h"
#include "Profiler.h"
#include "Disassembler.h"
#include "Z80.h"

Profiler::Profiler(const Z80& cpu)
: m_cpu(cpu) {
}

Profiler::~Profiler() {
}

void Profiler::addInstruction(uint8_t instruction) {
	m_instructions[instruction]++;
}

void Profiler::addAddress(uint16_t address) {
	m_addresses[address]++;
}

void Profiler::dump() const {
	dumpInstructionProfiles();
	dumpAddressProfiles();
}

void Profiler::dumpInstructionProfiles() const {
	std::cout << "** instructions" << std::endl;
	for (int i = 0; i < 0x100; ++i) {
		auto count = m_instructions[i];
		if (count > 0)
			std::cout << Disassembler::hex((uint8_t)i) << "\t" << count << std::endl;
	}
}

void Profiler::dumpAddressProfiles() const {
	std::cout << "** addresses" << std::endl;
	for (int i = 0; i < 0x10000; ++i) {
		auto count = m_addresses[i];
		if (count > 0) {
			uint16_t address = i;
			std::cout
				<< Disassembler::hex((uint16_t)i)
				<< '\t'
				<< m_disassembler.disassemble(m_cpu, address)
				<< '\n';
		}
	}
}
