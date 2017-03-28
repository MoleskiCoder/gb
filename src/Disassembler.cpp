#include "stdafx.h"
#include "Disassembler.h"

#include <sstream>
#include <iomanip>
#include <bitset>

#include "Memory.h"
#include "Z80.h"
#include "StatusFlags.h"

Disassembler::Disassembler() {
	// Disable exceptions where too many format arguments are available
	m_formatter.exceptions(boost::io::all_error_bits ^ boost::io::too_many_args_bit);
}

std::string Disassembler::state(const Z80& cpu) {

	auto pc = cpu.getProgramCounter();
	auto sp = cpu.getStackPointer();

	auto a = cpu.getA();
	auto f = cpu.getF();

	auto b = cpu.getBC().high;
	auto c = cpu.getBC().low;

	auto d = cpu.getDE().high;
	auto e = cpu.getDE().low;

	auto h = cpu.getHL().high;
	auto l = cpu.getHL().low;

	auto ix = cpu.getIX().word;
	auto iy = cpu.getIY().word;

	std::ostringstream output;

	output
		<< "PC=" << hex(pc)
		<< " "
		<< "SP=" << hex(sp)
		<< " " << "A=" << hex(a) << " " << "F=" << (std::string)f
		<< " " << "B=" << hex(b) << " " << "C=" << hex(c)
		<< " " << "D=" << hex(d) << " " << "E=" << hex(e)
		<< " " << "H=" << hex(h) << " " << "L=" << hex(l)
		<< " " << "IX=" << hex(ix) << " " << "IY=" << hex(iy);

	return output.str();
}

std::string Disassembler::disassemble(const Z80& cpu) const {

	std::ostringstream output;

	const auto& memory = cpu.getMemory();
	auto pc = cpu.getProgramCounter();
	auto opcode = memory.get(pc);

	// hex opcode
	output << hex(opcode);

	auto instruction = cpu.getInstructions()[opcode];
	auto isExtended = cpu.hasExtendedInstructions(opcode);
	if (isExtended) {
		auto surrogate = opcode;
		opcode = memory.get(++pc);
		instruction = cpu.getExtendedInstructions(surrogate)[opcode];
		output << hex(opcode);
	}

	auto immediate = memory.get(pc + 1);
	auto absolute = memory.getWord(pc + 1);
	auto relative = pc + (int8_t)immediate + 2;

	// hex raw operand
	switch (instruction.mode) {
	case Z80::Relative:
	case Z80::Immediate:
		output << hex(immediate);
		break;
	case Z80::Absolute:
		output << hex(memory.get(pc + 1));
		output << hex(memory.get(pc + 2));
		break;
	default:
		break;
	}
	output << "\t";

	m_formatter.parse(instruction.disassembly);
	output << m_formatter % (int)immediate % (int)absolute % relative;

	return output.str();
}

std::string Disassembler::hex(uint8_t value) {
	std::ostringstream output;
	output << std::hex << std::setw(2) << std::setfill('0') << (int)value;
	return output.str();
}

std::string Disassembler::hex(uint16_t value) {
	std::ostringstream output;
	output << std::hex << std::setw(4) << std::setfill('0') << (int)value;
	return output.str();
}

std::string Disassembler::binary(uint8_t value) {
	std::ostringstream output;
	output << std::bitset<8>(value);
	return output.str();
}

std::string Disassembler::invalid(uint8_t value) {
	std::ostringstream output;
	output << "Invalid instruction: " << hex(value) << "(" << binary(value) << ")";
	return output.str();
}