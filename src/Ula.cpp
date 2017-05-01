#include "stdafx.h"
#include "Ula.h"

Ula::Ula()
: m_linecntr(0),
  m_cas_out(Low),
  m_verticalRetrace(false),
  m_nmi(false) {
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
