#pragma once

class Ula {
public:
	enum SignalState {
		High, Low,
	};

	Ula();

	uint8_t& LINECNTR();
	SignalState& CAS_OUT();
	bool& VERTICAL_RETRACE();
	bool& NMI();

private:
	uint8_t m_linecntr;
	SignalState m_cas_out;
	bool m_verticalRetrace;
	bool m_nmi;
};
