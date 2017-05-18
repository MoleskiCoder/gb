#include "stdafx.h"
#include "Processor.h"

Processor::Processor(Bus& memory)
:	m_memory(memory),
	cycles(0),
	pc(0),
	sp(0xffff),
	m_halted(false) {}

void Processor::reset() {
	pc = 0;
}

void Processor::initialise() {
	sp = 0xffff;
	reset();
}

void Processor::pushWord(uint16_t value) {
	sp -= 2;
	setWord(sp, value);
}

uint16_t Processor::popWord() {
	auto value = getWord(sp);
	sp += 2;
	return value;
}
