#pragma once

#include <cstdint>
#include "Memory.h"

class Board;

class Ula : public Memory {
public:
	enum SignalState {
		High, Low,
	};

	Ula(Board& board);

	virtual uint8_t get(int address);

	uint8_t& LINECNTR();
	SignalState& CAS_OUT();
	bool& VERTICAL_RETRACE();
	bool& NMI();

	void incrementLineCounter();

private:
	Board& m_board;
	uint8_t m_linecntr;
	SignalState m_cas_out;
	bool m_verticalRetrace;
	bool m_nmi;
};
