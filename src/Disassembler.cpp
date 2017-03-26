#include "stdafx.h"
#include "Disassembler.h"

#include <sstream>
#include <iomanip>
#include <bitset>

#include "Memory.h"
#include "Z80.h"
#include "StatusFlags.h"

Disassembler::Disassembler() {
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

	std::ostringstream output;

	output
		<< "PC=" << hex(pc)
		<< " "
		<< "SP=" << hex(sp)
		<< " " << "A=" << hex(a) << " " << "F=" << (std::string)f
		<< " " << "B=" << hex(b) << " " << "C=" << hex(c)
		<< " " << "D=" << hex(d) << " " << "E=" << hex(e)
		<< " " << "H=" << hex(h) << " " << "L=" << hex(l);

	return output.str();
}

std::string Disassembler::disassemble(const Z80& cpu) {

	const auto& memory = cpu.getMemory();
	auto pc = cpu.getProgramCounter();
	auto opcode = memory.get(pc);
	const auto& instruction = cpu.getInstructions()[opcode];

	std::ostringstream output;

	// hex opcode
	output << hex(opcode);

	// hex raw operand
	switch (instruction.mode) {
	case Z80::Immediate:
		output << hex(memory.get(pc + 1));
		break;
	case Z80::Absolute:
		output << hex(memory.get(pc + 1));
		output << hex(memory.get(pc + 2));
		break;
	default:
		break;
	}
	output << "\t";

	// base disassembly
	output << instruction.disassembly;

	// disassembly operand
	switch (instruction.mode) {
	case Z80::Immediate:
		output << hex(memory.get(pc + 1));
		break;
	case Z80::Absolute:
		output << hex(memory.getWord(pc + 1));
		break;
	default:
		break;
	}

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