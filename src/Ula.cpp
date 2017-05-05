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

uint8_t Ula::get(const uint16_t address) {

	ADDRESS() = address;

	if ((Processor::Bit15 & m_address) && m_board.getCPU().getM1()) {

		auto mirror = m_address & ~(Processor::Bit15);
		auto value = m_board.BUS().peek(mirror);

		// Halt?  (AKA newline)
		if (value == 0x76) {
			restartRasterLine();
			return value;
		}

		auto outputByte = getPixelByteOffset(m_rasterX++, m_rasterY);

		if (value & Processor::Bit6) {

			// display white
			m_pixels[outputByte] = 0;

		} else {

			auto invert = (value & Processor::Bit7) != 0;
			auto character = value & Processor::Mask5;

			auto cpu = m_board.getCPUMutable();
			auto definition = cpu.IV() * 0x100 + character * 8 + LINECNTR();

			auto line = m_board.BUS().get(definition);
			m_pixels[outputByte] = invert ? ~line : line;

			return 0;
		}
	}

	return Memory::reference();
}

void Ula::incrementLineCounter() {
	LINECNTR() = ((LINECNTR() & Processor::Mask3) + 1) & Processor::Mask3;
	restartRasterLine();
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
