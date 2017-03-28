#pragma once

#include <string>
#include <cstdint>

struct StatusFlags {

	bool S;
	bool Z;
	bool HC;
	bool PV;
	bool N;
	bool C;

	enum StatusBits {
		Sign = 0x80,				// S
		Zero = 0x40,				// Z
		HalfCarry = 0x10,			// HC
		Parity = 0x4,				// P
		Overflow = 0x4,				// V
		Subtract = 0x2,				// N
		Carry = 0x1,				// S
	};

	StatusFlags(uint8_t value = 0) {
		S = (value & StatusBits::Sign) != 0;
		Z = (value & StatusBits::Zero) != 0;
		HC = (value & StatusBits::HalfCarry) != 0;
		PV = (value & StatusBits::Parity) != 0;		// parity/overflow
		N = (value & StatusBits::Subtract) != 0;
		C = (value & StatusBits::Carry) != 0;
	}

	operator uint8_t() const {

		uint8_t flags = 0;

		if (S)
			flags |= StatusBits::Sign;

		if (Z)
			flags |= StatusBits::Zero;

		flags &= ~0x20;		// Reserved off

		if (HC)
			flags |= StatusBits::HalfCarry;

		flags &= ~0x8;		// Reserved off

		if (PV)
			flags |= StatusBits::Parity;

		if (N)
			flags |= StatusBits::Subtract;

		if (C)
			flags |= StatusBits::Carry;

		return flags;
	}

	operator std::string() const {
		std::string returned;
		returned += S ? "S" : "-";
		returned += Z ? "Z" : "-";
		returned += "0";
		returned += HC ? "A" : "-";
		returned += "0";
		returned += PV ? "P" : "-";
		returned += N ? "N" : "-";
		returned += C ? "C" : "-";
		return returned;
	}
};
