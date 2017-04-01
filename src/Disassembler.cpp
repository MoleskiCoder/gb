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
	auto a_alt = cpu.getA_Alt();
	auto f = cpu.getF();
	auto f_alt = cpu.getF_Alt();

	auto b = cpu.getBC().high;
	auto c = cpu.getBC().low;
	auto b_alt = cpu.getBC_Alt().high;
	auto c_alt = cpu.getBC_Alt().low;

	auto d = cpu.getDE().high;
	auto e = cpu.getDE().low;
	auto d_alt = cpu.getDE_Alt().high;
	auto e_alt = cpu.getDE_Alt().low;

	auto h = cpu.getHL().high;
	auto l = cpu.getHL().low;
	auto h_alt = cpu.getHL_Alt().high;
	auto l_alt = cpu.getHL_Alt().low;

	auto ix = cpu.getIX().word;
	auto iy = cpu.getIY().word;

	std::ostringstream output;

	output
		<< "IX=" << hex(ix)
		<< " "
		<< "IY=" << hex(iy) << "\t"
		<< "A'=" << hex(a_alt) << " " << "F'=" << (std::string)f_alt
		<< " " << "B'=" << hex(b_alt) << " " << "C'=" << hex(c_alt)
		<< " " << "D'=" << hex(d_alt) << " " << "E'=" << hex(e_alt)
		<< " " << "H'=" << hex(h_alt) << " " << "L'=" << hex(l_alt)
		<< std::endl
		<< "PC=" << hex(pc)
		<< " "
		<< "SP=" << hex(sp) << "\t"
		<< "A =" << hex(a) << " " << "F =" << (std::string)f
		<< " " << "B =" << hex(b) << " " << "C =" << hex(c)
		<< " " << "D =" << hex(d) << " " << "E =" << hex(e)
		<< " " << "H =" << hex(h) << " " << "L =" << hex(l);


	return output.str();
}

std::string Disassembler::disassemble(const Z80& cpu, uint16_t& pc) const {

	std::ostringstream output;

	const auto& memory = cpu.getMemory();
	auto opcode = memory.get(pc++);

	// hex opcode
	output << hex(opcode);

	auto instruction = cpu.getInstructions()[opcode];
	auto isExtended = cpu.hasExtendedInstructions(opcode);
	if (isExtended) {
		auto surrogate = opcode;
		opcode = memory.get(pc++);
		instruction = cpu.getExtendedInstructions(surrogate)[opcode];
		output << hex(opcode);
	}

	auto immediate = memory.get(pc);
	auto absolute = memory.getWord(pc);
	auto displacement = (int8_t)immediate;
	auto relative = pc + displacement + 2;
	auto indexedImmediate = memory.get(pc + 1);

	// hex raw operand
	switch (instruction.mode) {
	case Z80::Relative:
	case Z80::Immediate:
		output << hex(immediate);
		++pc;
		break;
	case Z80::Absolute:
		output << hex(memory.get(pc));
		output << hex(memory.get(pc + 1));
		pc += 2;
		break;
	default:
		break;
	}
	output << "\t";

	m_formatter.parse(instruction.disassembly);
	output << m_formatter % (int)immediate % (int)absolute % relative % displacement % indexedImmediate;

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