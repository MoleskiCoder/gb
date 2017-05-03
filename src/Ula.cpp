#include "stdafx.h"
#include "Ula.h"
#include "Board.h"

Ula::Ula(Board& board)
: Memory(0xffff),
  m_board(board),
  m_linecntr(0),
  m_cas_out(Low),
  m_verticalRetrace(false),
  m_nmi(false) {
}

uint8_t Ula::get(int address) {
	if ((Processor::Bit15 & address) && m_board.getCPU().getM1()) {

		ADDRESS() = address & ~(Processor::Bit15);
		auto value = reference();

		if (value & Processor::Bit6) {

			// display white

		} else {

			auto invert = (value & Processor::Bit7) != 0;
			auto character = value & Processor::Mask5;

			auto cpu = m_board.getCPUMutable();
			auto definition = cpu.IV() * 0x100 + character * 8 + LINECNTR();

			for (int i = 0; i < 8; ++i) {
				auto bit = 1 << i;
			}

			return 0;
		}
	} else {
		ADDRESS() = address;
	}
	return Memory::reference();
}

void Ula::incrementLineCounter() {
	LINECNTR() = ((LINECNTR() & Processor::Mask3) + 1) & Processor::Mask3;
}

uint8_t& Ula::LINECNTR() {
	return m_linecntr;
}

Ula::SignalState& Ula::CAS_OUT() {
	return m_cas_out;
}

bool& Ula::VERTICAL_RETRACE() {
	return m_verticalRetrace;
}

bool& Ula::NMI() {
	return m_nmi;
}
