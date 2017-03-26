#pragma once

#include <string>
#include <boost/format.hpp>

class Z80;

class Disassembler {
public:
	Disassembler();

	static std::string state(const Z80& cpu);

	static std::string hex(uint8_t value);
	static std::string hex(uint16_t value);
	static std::string binary(uint8_t value);

	static std::string invalid(uint8_t value);

	std::string disassemble(const Z80& cpu) const;

private:
	mutable boost::format m_formatter;
};
