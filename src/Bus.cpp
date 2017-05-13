#include "stdafx.h"
#include "Bus.h"

Bus::Bus()
: Memory(0xffff) {
}

void Bus::reset() {
	REG_NR52() = 0xf1;
	REG_LCDC() = 0x91;
}