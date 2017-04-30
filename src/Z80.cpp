#include "stdafx.h"
#include "Z80.h"

// based on http://www.z80.info/decoding.htm
// Half carry flag help from https://github.com/oubiwann/z80

Z80::Z80(Memory& memory, InputOutput& ports)
: Processor(memory, ports),
  m_registerSet(0),
  m_accumulatorFlagsSet(0),
  m_refresh(0xff),
  iv(0xff),
  m_interruptMode(0),
  m_iff1(false),
  m_iff2(false),
  m_prefixCB(false),
  m_prefixDD(false),
  m_prefixED(false),
  m_prefixFD(false) {
	m_ix.word = 0xffff;
	m_iy.word = 0xffff;
	m_memptr.word = 0;
}

void Z80::reset() {
	Processor::reset();
	IFF1() = IFF2() = false;
}

void Z80::initialise() {

	Processor::initialise();

	IM() = 0;

	m_registerSet = 1;
	m_accumulatorFlagsSet = 1;

	AF().word = 0xffff;
	BC().word = 0xffff;
	DE().word = 0xffff;
	HL().word = 0xffff;

	m_registerSet = 1;
	m_accumulatorFlagsSet = 1;

	AF().word = 0xffff;
	BC().word = 0xffff;
	DE().word = 0xffff;
	HL().word = 0xffff;

	IX().word = 0xffff;
	IY().word = 0xffff;

	REFRESH() = 0xff;
	IV() = 0xff;
	MEMPTR().word = 0;

	m_prefixCB = false;
	m_prefixDD = false;
	m_prefixED = false;
	m_prefixFD = false;
}

void Z80::disableInterrupts() {
	IFF1() = IFF2() = false;

}

void Z80::enableInterrupts() {
	IFF1() = IFF2() = true;
}

void Z80::interrupt(bool maskable, uint8_t value) {
	if (!maskable || (maskable && IFF1())) {
		maskable ? disableInterrupts() : IFF1() = 0;
		switch (IM()) {
		case 0:
			execute(value);
			break;
		case 1:
			restart(7);
			cycles += 13;
			break;
		case 2:
			pushWord(pc);
			pc = makeWord(value, IV());
			cycles += 19;
			break;
		}
	}
}

void Z80::adjustSign(uint8_t value) {
	setFlag(SF, value & SF);
}

void Z80::adjustZero(uint8_t value) {
	clearFlag(ZF, value);
}

void Z80::adjustParity(uint8_t value) {
	static const uint8_t lookup[0x10] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
	auto set = (lookup[highNibble(value)] + lookup[lowNibble(value)]);
	auto even = (set % 2) == 0;
	setFlag(PF, even);
}

void Z80::adjustSZP(uint8_t value) {
	adjustSign(value);
	adjustZero(value);
	adjustParity(value);
}

void Z80::adjustXYFlags(uint8_t value) {
	setFlag(XF, value & XF);
	setFlag(YF, value & YF);
}

void Z80::postIncrement(uint8_t value) {
	adjustSign(value);
	adjustZero(value);
	adjustXYFlags(value);
	clearFlag(NF);
	setFlag(VF, value == Bit7);
	clearFlag(HC, lowNibble(value));
}

void Z80::postDecrement(uint8_t value) {
	adjustSign(value);
	adjustZero(value);
	adjustXYFlags(value);
	setFlag(NF);
	setFlag(VF, value == Mask7);
	clearFlag(HC, lowNibble(value + 1));
}

void Z80::restart(uint8_t position) {
	uint16_t address = position << 3;
	pushWord(pc);
	setPcViaMemptr(address);
}

void Z80::jrConditional(int conditional) {
	auto offset = (int8_t)fetchByte();
	if (conditional) {
		setPcViaMemptr(pc + offset);
		cycles += 5;
	}
}

void Z80::jrConditionalFlag(int flag) {
	switch (flag) {
	case 0:	// NZ
		jrConditional(!(F() & ZF));
		break;
	case 1:	// Z
		jrConditional(F() & ZF);
		break;
	case 2:	// NC
		jrConditional(!(F() & CF));
		break;
	case 3:	// C
		jrConditional(F() & CF);
		break;
	case 4:	// PO
		jrConditional(!(F() & PF));
		break;
	case 5:	// PE
		jrConditional(F() & PF);
		break;
	case 6:	// P
		jrConditional(!(F() & SF));
		break;
	case 7:	// M
		jrConditional(F() & SF);
		break;
	}
}

void Z80::jumpConditional(int conditional) {
	auto address = fetchWord();
	if (conditional) {
		pc = address;
	}
	MEMPTR().word = address;
}

void Z80::jumpConditionalFlag(int flag) {
	switch (flag) {
	case 0:	// NZ
		jumpConditional(!(F() & ZF));
		break;
	case 1:	// Z
		jumpConditional(F() & ZF);
		break;
	case 2:	// NC
		jumpConditional(!(F() & CF));
		break;
	case 3:	// C
		jumpConditional(F() & CF);
		break;
	case 4:	// PO
		jumpConditional(!(F() & PF));
		break;
	case 5:	// PE
		jumpConditional(F() & PF);
		break;
	case 6:	// P
		jumpConditional(!(F() & SF));
		break;
	case 7:	// M
		jumpConditional(F() & SF);
		break;
	}
}

void Z80::ret() {
	setPcViaMemptr(popWord());
}

void Z80::retn() {
	ret();
	IFF1() = IFF2();
}

void Z80::reti() {
	retn();
}

void Z80::returnConditional(int condition) {
	if (condition) {
		ret();
		cycles += 6;
	}
}

void Z80::returnConditionalFlag(int flag) {
	switch (flag) {
	case 0:	// NZ
		returnConditional(!(F() & ZF));
		break;
	case 1:	// Z
		returnConditional(F() & ZF);
		break;
	case 2:	// NC
		returnConditional(!(F() & CF));
		break;
	case 3:	// C
		returnConditional(F() & CF);
		break;
	case 4:	// PO
		returnConditional(!(F() & PF));
		break;
	case 5:	// PE
		returnConditional(F() & PF);
		break;
	case 6:	// P
		returnConditional(!(F() & SF));
		break;
	case 7:	// M
		returnConditional(F() & SF);
		break;
	}
}

void Z80::call(uint16_t address) {
	pushWord(pc + 2);
	pc = address;
}

void Z80::callConditional(uint16_t address, int condition) {
	if (condition) {
		call(address);
		cycles += 7;
	} else {
		pc += 2;
	}
	MEMPTR().word = address;
}

void Z80::callConditionalFlag(uint16_t address, int flag) {
	switch (flag) {
	case 0:	// NZ
		callConditional(address, !(F() & ZF));
		break;
	case 1:	// Z
		callConditional(address, F() & ZF);
		break;
	case 2:	// NC
		callConditional(address, !(F() & CF));
		break;
	case 3:	// C
		callConditional(address, F() & CF);
		break;
	case 4:	// PO
		callConditional(address, !(F() & PF));
		break;
	case 5:	// PE
		callConditional(address, F() & PF);
		break;
	case 6:	// P
		callConditional(address, !(F() & SF));
		break;
	case 7:	// M
		callConditional(address, F() & SF);
		break;
	}
}

uint16_t Z80::sbc(uint16_t value) {

	auto hl = RP(HL_IDX);

	auto negativeHL = (hl & Bit15) != 0;
	auto negativeValue = (value & Bit15) != 0;
	auto high = Memory::highByte(hl);
	auto highValue = Memory::highByte(value);
	auto applyCarry = F() & CF;

	uint32_t result = (int)hl - (int)value;
	if (applyCarry)
		--result;
	auto negativeResult = (result & Bit15) != 0;
	auto highResult = Memory::highByte(result);

	setFlag(SF, negativeResult);
	clearFlag(ZF, result);
	adjustHalfCarrySub(high, highValue, highResult);

	auto sameOperandSign = negativeHL == negativeValue;
	auto overflow = false;
	if (!sameOperandSign) {
		overflow = (negativeHL != negativeResult);
	}
	setFlag(VF, overflow);

	setFlag(NF);
	setFlag(CF, result & Bit16);

	adjustXYFlags(highResult);

	return result;
}

uint16_t Z80::adc(uint16_t value) {

	auto hl = RP(HL_IDX);

	auto negativeHL = (hl & Bit15) != 0;
	auto negativeValue = (value & Bit15) != 0;
	auto high = Memory::highByte(hl);
	auto highValue = Memory::highByte(value);
	auto applyCarry = F() & CF;

	uint32_t result = (int)hl + (int)value;
	if (applyCarry)
		++result;
	auto negativeResult = (result & Bit15) != 0;
	auto highResult = Memory::highByte(result);

	setFlag(SF, negativeResult);
	clearFlag(ZF, result);
	adjustHalfCarryAdd(high, highValue, highResult);

	auto sameOperandSign = negativeHL == negativeValue;
	auto overflow = false;
	if (sameOperandSign) {
		overflow = (negativeHL != negativeResult);
	}
	setFlag(VF, overflow);

	clearFlag(NF);
	setFlag(CF, result & Bit16);

	adjustXYFlags(highResult);

	return result;
}

uint16_t Z80::add(uint16_t value) {

	auto hl = RP(HL_IDX);

	auto high = Memory::highByte(hl);
	auto highValue = Memory::highByte(value);

	uint32_t result = (int)hl + (int)value;

	auto highResult = Memory::highByte(result);

	clearFlag(NF);
	setFlag(CF, result & Bit16);
	adjustHalfCarryAdd(high, highValue, highResult);

	adjustXYFlags(highResult);

	return result;
}

uint8_t Z80::sbc(uint8_t value) {

	uint16_t result = A() - value;
	if (F() & CF)
		--result;

	setFlag(SF, result & Bit7);
	clearFlag(ZF, result);
	adjustHalfCarrySub(A(), value, result);

	auto beforeNegative = A() & Bit7;
	auto valueNegative = value & Bit7;
	auto sameOperandSign = beforeNegative == valueNegative;
	auto overflow = false;
	if (!sameOperandSign) {
		auto afterNegative = result & Bit7;
		overflow = (beforeNegative != afterNegative);
	}
	setFlag(VF, overflow);

	setFlag(NF);
	setFlag(CF, result & Bit8);

	auto returned = (uint8_t)result;
	adjustXYFlags(returned);

	return returned;
}

uint8_t Z80::sub(uint8_t value) {

	uint16_t result = A() - value;

	setFlag(SF, result & Bit7);
	clearFlag(ZF, result);
	adjustHalfCarrySub(A(), value, result);

	auto beforeNegative = A() & Bit7;
	auto valueNegative = value & Bit7;
	auto sameOperandSign = beforeNegative == valueNegative;
	auto overflow = false;
	if (!sameOperandSign) {
		auto afterNegative = result & Bit7;
		overflow = (beforeNegative != afterNegative);
	}
	setFlag(VF, overflow);

	setFlag(NF);
	setFlag(CF, result & Bit8);

	auto returned = (uint8_t)result;
	adjustXYFlags(returned);
	return returned;
}

uint8_t Z80::adc(uint8_t value) {

	uint16_t result = A() + value;
	if (F() & CF)
		++result;

	setFlag(SF, result & Bit7);
	clearFlag(ZF, result);
	adjustHalfCarryAdd(A(), value, result);

	auto beforeNegative = A() & Bit7;
	auto valueNegative = value & Bit7;
	auto sameOperandSign = beforeNegative == valueNegative;
	auto overflow = false;
	if (sameOperandSign) {
		auto afterNegative = result & Bit7;
		overflow = (beforeNegative != afterNegative);
	}
	setFlag(VF, overflow);

	clearFlag(NF);
	setFlag(CF, result & Bit8);

	auto returned = (uint8_t)result;
	adjustXYFlags(returned);
	return returned;
}

uint8_t Z80::add(uint8_t value) {

	uint16_t result = A() + value;

	setFlag(SF, result & Bit7);
	clearFlag(ZF, result);
	adjustHalfCarryAdd(A(), value, result);

	auto beforeNegative = A() & Bit7;
	auto valueNegative = value & Bit7;
	auto sameOperandSign = beforeNegative == valueNegative;
	auto overflow = false;
	if (sameOperandSign) {
		auto afterNegative = result & Bit7;
		overflow = (beforeNegative != afterNegative);
	}
	setFlag(VF, overflow);

	clearFlag(NF);
	setFlag(CF, result & Bit8);

	auto returned = (uint8_t)result;
	adjustXYFlags(returned);
	return returned;
}

//

void Z80::andr(uint8_t& operand, uint8_t value) {
	setFlag(HC);
	clearFlag(CF | NF);
	operand &= value;
	adjustSZP(operand);
	adjustXYFlags(operand);
}

void Z80::anda(uint8_t value) {
	andr(A(), value);
}

void Z80::xora(uint8_t value) {
	clearFlag(HC | CF | NF);
	A() ^= value;
	adjustSZP(A());
	adjustXYFlags(A());
}

void Z80::ora(uint8_t value) {
	clearFlag(HC | CF | NF);
	A() |= value;
	adjustSZP(A());
	adjustXYFlags(A());
}

void Z80::compare(uint8_t value) {
	sub(value);
	adjustXYFlags(value);
}

//

void Z80::rlc(uint8_t& operand) {
	auto carry = operand & Bit7;
	operand <<= 1;
	setFlag(CF, carry);
	carry ? operand |= Bit0 : operand &= ~Bit0;
	clearFlag(NF | HC);
	adjustXYFlags(operand);
}

void Z80::rrc(uint8_t& operand) {
	auto carry = operand & Bit0;
	operand >>= 1;
	carry ? operand |= Bit7 : operand &= ~Bit7;
	setFlag(CF, carry);
	clearFlag(NF | HC);
	adjustXYFlags(operand);
}

void Z80::rl(uint8_t& operand) {
	auto oldCarry = F() & CF;
	auto newCarry = operand & Bit7;
	operand <<= 1;
	oldCarry ? operand |= Bit0 : operand &= ~Bit0;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustXYFlags(operand);
}

void Z80::rr(uint8_t& operand) {
	auto oldCarry = F() & CF;
	auto newCarry = operand & Bit0;
	operand >>= 1;
	operand |= oldCarry << 7;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustXYFlags(operand);
}

//

void Z80::sla(uint8_t& operand) {
	auto newCarry = operand & Bit7;
	operand <<= 1;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustXYFlags(operand);
}

void Z80::sra(uint8_t& operand) {
	auto new7 = operand & Bit7;
	auto newCarry = operand & Bit0;
	operand >>= 1;
	operand |= new7;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustXYFlags(operand);
}

void Z80::sll(uint8_t& operand) {
	auto newCarry = operand & Bit7;
	operand <<= 1;
	operand |= 1;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustXYFlags(operand);
}

void Z80::srl(uint8_t& operand) {
	auto newCarry = operand & Bit0;
	operand >>= 1;
	operand &= ~Bit7;	// clear bit 7
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustXYFlags(operand);
	setFlag(ZF, operand);
}

//

void Z80::rlca() {
	rlc(A());
}

void Z80::rrca() {
	rrc(A());
}

void Z80::rla() {
	rl(A());
}

void Z80::rra() {
	rr(A());
}

//

void Z80::bit(int n, uint8_t& operand) {
	auto carry = F() & CF;
	uint8_t discarded = operand;
	andr(discarded, 1 << n);
	clearFlag(PF, discarded);
	setFlag(CF, carry);
}

void Z80::res(int n, uint8_t& operand) {
	auto bit = 1 << n;
	operand &= ~bit;
}

void Z80::set(int n, uint8_t& operand) {
	auto bit = 1 << n;
	operand |= bit;
}

//

void Z80::daa() {

	uint8_t a = A();

	auto lowAdjust = (F() & HC) | ((A() & 0xf) > 9);
	auto highAdjust = (F() & CF) | (A() > 0x99);

	if (F() & NF) {
		if (lowAdjust)
			a -= 6;
		if (highAdjust)
			a -= 0x60;
	} else {
		if (lowAdjust)
			a += 6;
		if (highAdjust)
			a += 0x60;
	}

	F() = (F() & (CF | NF)) | (A() > 0x99) | ((A() ^ a) & HC);

	adjustSZP(a);
	adjustXYFlags(a);

	A() = a;
}

void Z80::cpl() {
	A() = ~A();
	adjustXYFlags(A());
	setFlag(HC | NF);
}

void Z80::scf() {
	setFlag(CF);
	adjustXYFlags(A());
	clearFlag(HC | NF);
}

void Z80::ccf() {
	auto carry = F() & CF;
	setFlag(HC, carry);
	clearFlag(CF, carry);
	clearFlag(NF);
	adjustXYFlags(A());
}

void Z80::xhtl(register16_t& operand) {
	auto tos = getWord(sp);
	setWord(sp, operand.word);
	MEMPTR().word = operand.word = tos;
}

void Z80::xhtl() {
	if (m_prefixDD)
		xhtl(IX());
	else if (m_prefixFD)
		xhtl(IY());
	else
		xhtl(HL());
}

void Z80::cp(uint16_t source) {
	auto value = m_memory.get(source);
	uint8_t result = A() - value;

	setFlag(PF, --BC().word);

	setFlag(SF, result & Bit7);
	clearFlag(ZF, result);
	adjustHalfCarrySub(A(), value, result);
	setFlag(NF);

	if (F() & HC)
		result -= 1;

	setFlag(YF, result & Bit1);
	setFlag(XF, result & Bit3);
}

void Z80::cpi() {
	cp(HL().word++);
	MEMPTR().word++;
}

void Z80::cpd() {
	cp(HL().word--);
	MEMPTR().word--;
}

void Z80::blockLoad(uint16_t source, uint16_t destination) {
	auto value = m_memory.get(source);
	m_memory.set(destination, value);
	auto xy = A() + value;
	setFlag(XF, xy & 8);
	setFlag(YF, xy & 2);
	clearFlag(NF | HC);
	setFlag(PF, --BC().word);
}

void Z80::ldd() {
	auto source = HL().word--;
	auto destination = DE().word--;
	blockLoad(source, destination);
}

void Z80::ldi() {
	auto source = HL().word++;
	auto destination = DE().word++;
	blockLoad(source, destination);
}

void Z80::ldir() {

	auto bc = BC().word;

	ldi();
	if (F() & PF) {		// See LDI
		cycles += 5;
		pc -= 2;
	}

	if (bc != 1)
		MEMPTR().word = pc + 1;
}

void Z80::lddr() {

	auto bc = BC().word;

	ldd();
	if (F() & PF) {		// See LDR
		cycles += 5;
		pc -= 2;
	}

	if (bc != 1)
		MEMPTR().word = pc + 1;
}

void Z80::cpir() {
	cpi();
	if ((F() & PF) && ((F() & ZF) == 0)) {		// See CPI
		cycles += 5;
		pc -= 2;
	}
	MEMPTR().word = pc + 1;
}

void Z80::cpdr() {
	cpd();
	if ((F() & PF) && ((F() & ZF) == 0)) {		// See CPD
		cycles += 5;
		pc -= 2;
	}
	MEMPTR().word = pc + 1;
}

void Z80::ini() {
	auto bc = BC().word;
	auto port = C();
	auto value = m_ports.read(port);
	auto address = HL().word;
	m_memory.set(address, value);
	postDecrement(--B());
	setFlag(NF);
	HL().word++;
	MEMPTR().word = bc + 1;
}

void Z80::inir() {
	ini();
	if ((F() & ZF) != 0) {		// See INI
		cycles += 5;
		pc -= 2;
	}
}

void Z80::ind() {
	auto bc = BC().word;
	auto port = C();
	auto value = m_ports.read(port);
	auto address = HL().word;
	m_memory.set(address, value);
	postDecrement(--B());
	setFlag(NF, value & Bit7);
	HL().word--;
	setFlag(HC | CF, (value + ((C() - 1) & 0xff) > 0xff));
	adjustParity(((value + ((C() - 1) & 0xff)) & 7) ^ B());
	MEMPTR().word = bc - 1;
}

void Z80::indr() {
	ind();
	if ((F() & ZF) != 0) {		// See IND
		cycles += 5;
		pc -= 2;
	}
}

void Z80::outi() {
	auto address = HL().word;
	auto value = m_memory.get(address);
	auto port = C();
	m_ports.write(port, value);
	postDecrement(--B());
	setFlag(NF, value & Bit7);
	HL().word++;
	setFlag(HC | CF, (L() + value) > 0xff);
	adjustParity(((value + L()) & 7) ^ B());
	MEMPTR().word = BC().word + 1;
}

void Z80::otir() {
	outi();
	if ((F() & ZF) != 0) {		// See OUTI
		cycles += 5;
		pc -= 2;
	}
}

void Z80::outd() {
	auto address = HL().word;
	auto value = m_memory.get(address);
	auto port = C();
	m_ports.write(port, value);
	postDecrement(--B());
	setFlag(NF, value & Bit7);
	HL().word++;
	setFlag(HC | CF, (L() + value) > 0xff);
	adjustParity(((value + L()) & 7) ^ B());
	MEMPTR().word = BC().word - 1;
}

void Z80::otdr() {
	outd();
	if ((F() & ZF) != 0) {		// See OUTD
		cycles += 5;
		pc -= 2;
	}
}

void Z80::neg() {
	auto original = A();
	A() = 0;
	A() = sub(original);
	setFlag(PF, original == 0x80);
	setFlag(CF, original != 0);
}

void Z80::rrd() {
	auto accumulator = A();
	auto memory = m_memory.get(HL().word);
	A() = (accumulator & 0xf0) | lowNibble(memory);
	uint8_t updated = promoteNibble(lowNibble(accumulator)) | highNibble(memory);
	m_memory.set(HL().word, updated);
	adjustSZP(A());
	adjustXYFlags(A());
	clearFlag(NF | HC);
	MEMPTR().word = HL().word + 1;
}

void Z80::rld() {
	auto accumulator = A();
	auto memory = m_memory.get(HL().word);
	uint8_t updated = lowNibble(accumulator) | promoteNibble(memory);
	A() = (accumulator & 0xf0) | highNibble(memory);
	m_memory.set(HL().word, updated);
	adjustSZP(A());
	adjustXYFlags(A());
	clearFlag(NF | HC);
	MEMPTR().word = HL().word + 1;
}

void Z80::readPort(uint8_t& operand, uint8_t port) {
	auto bc = BC().word;
	operand = m_ports.read(port);
	adjustSZP(operand);
	adjustXYFlags(operand);
	clearFlag(HC | NF);
	MEMPTR().word = bc + 1;
}

void Z80::step() {
	ExecutingInstruction.fire(*this);
	m_prefixCB = m_prefixDD = m_prefixED = m_prefixFD = false;
	execute(fetchByte());
}

void Z80::execute(uint8_t opcode) {

	REFRESH()++;

	auto x = (opcode & 0b11000000) >> 6;
	auto y = (opcode & 0b111000) >> 3;
	auto z = (opcode & 0b111);

	auto p = (y & 0b110) >> 1;
	auto q = (y & 1);

	auto oldCycles = cycles;

	if (m_prefixCB)
		executeCB(x, y, z, p, q);
	else if (m_prefixED)
		executeED(x, y, z, p, q);
	else
		executeOther(x, y, z, p, q);

	auto newCycles = cycles;
	if (newCycles == oldCycles)
		throw std::logic_error("Unhandled opcode");
}

void Z80::executeCB(int x, int y, int z, int p, int q) {
	switch (x) {
	case 0:	// rot[y] r[z]
		switch (y) {
		case 0:
			if (m_prefixDD || m_prefixFD) {
				auto& displaced = DISPLACED();
				uint8_t result = displaced;
				rlc(result);
				displaced = result;
				R(z, false) = result;
			} else {
				rlc(R(z));
			}
			break;
		case 1:
			if (m_prefixDD || m_prefixFD) {
				auto& displaced = DISPLACED();
				uint8_t result = displaced;
				rrc(result);
				displaced = result;
				R(z, false) = result;
			} else {
				rrc(R(z));
			}
			break;
		case 2:
			if (m_prefixDD || m_prefixFD) {
				auto& displaced = DISPLACED();
				uint8_t result = displaced;
				rl(result);
				displaced = result;
				R(z, false) = result;
			} else {
				rl(R(z));
			}
			break;
		case 3:
			if (m_prefixDD || m_prefixFD) {
				auto& displaced = DISPLACED();
				uint8_t result = displaced;
				rr(result);
				displaced = result;
				R(z, false) = result;
			} else {
				rr(R(z));
			}
			break;
		case 4:
			if (m_prefixDD || m_prefixFD) {
				auto& displaced = DISPLACED();
				uint8_t result = displaced;
				sla(result);
				displaced = result;
				R(z, false) = result;
			} else {
				sla(R(z));
			}
			break;
		case 5:
			if (m_prefixDD || m_prefixFD) {
				auto& displaced = DISPLACED();
				uint8_t result = displaced;
				sra(result);
				displaced = result;
				R(z, false) = result;
			} else {
				sra(R(z));
			}
			break;
		case 6:
			if (m_prefixDD || m_prefixFD) {
				auto& displaced = DISPLACED();
				uint8_t result = displaced;
				sll(result);
				displaced = result;
				R(z, false) = result;
			} else {
				sll(R(z));
			}
			break;
		case 7:
			if (m_prefixDD || m_prefixFD) {
				auto& displaced = DISPLACED();
				uint8_t result = displaced;
				srl(result);
				displaced = result;
				R(z, false) = result;
			} else {
				srl(R(z));
			}
			break;
		}
		if (m_prefixDD || m_prefixFD)
			adjustSZP(DISPLACED());
		else
			adjustSZP(R(z));
		if (m_prefixDD || m_prefixFD) {
			cycles += 23;
		} else {
			cycles += 8;
			if (z == 6)
				cycles += 7;
		}
		break;
	case 1:	// BIT y, r[z]
		if (m_prefixDD || m_prefixFD) {
			auto& displaced = DISPLACED();
			auto memptr = MEMPTR().word;
			bit(y, displaced);
			MEMPTR().word = memptr;
			adjustXYFlags(MEMPTR().high);
			cycles += 20;
		} else {
			auto operand = R(z);
			bit(y, operand);
			cycles += 8;
			if (z == 6) {
				adjustXYFlags(MEMPTR().high);
				cycles += 4;
			} else {
				adjustXYFlags(operand);
			}
		}
		break;
	case 2:	// RES y, r[z]
		if (m_prefixDD || m_prefixFD) {
			auto& displaced = DISPLACED();
			uint8_t result = displaced;
			res(y, result);
			displaced = result;
			R(z, false) = result;
			cycles += 23;
		} else {
			res(y, R(z));
			cycles += 8;
			if (z == 6)
				cycles += 7;
		}
		break;
	case 3:	// SET y, r[z]
		if (m_prefixDD || m_prefixFD) {
			auto& displaced = DISPLACED();
			uint8_t result = displaced;
			set(y, result);
			displaced = result;
			R(z, false) = result;
			cycles += 23;
		} else {
			set(y, R(z));
			cycles += 8;
			if (z == 6)
				cycles += 7;
		}
		break;
	}
}

void Z80::executeED(int x, int y, int z, int p, int q) {
	switch (x) {
	case 0:
	case 3:	// Invalid instruction, equivalent to NONI followed by NOP
		cycles += 8;
		break;
	case 1:
		switch (z) {
		case 0:	// Input from port with 16-bit address
			if (y == 6) {	// IN (C)
				uint8_t value;
				readPort(value, C());
			} else {		// IN r[y],(C)
				readPort(R(y), C());
			}
			cycles += 12;
			break;
		case 1:	// Output to port with 16-bit address
			if (y == 6)	// OUT (C),0
				m_ports.write(C(), 0);
			else		// OUT (C),r[y]
				m_ports.write(C(), R(y));
			MEMPTR().word = BC().word + 1;
			cycles += 12;
			break;
		case 2:	// 16-bit add/subtract with carry
			switch (q) {
			case 0:	// SBC HL, rp[p]
				sbcViaMemptr(RP(HL_IDX), RP(p));
				break;
			case 1:	// ADC HL, rp[p]
				adcViaMemptr(RP(HL_IDX), RP(p));
				break;
			}
			cycles += 15;
			break;
		case 3:	// Retrieve/store register pair from/to immediate address
			switch (q) {
			case 0:	// LD (nn), rp[p]
				setWordViaMemptr(fetchWord(), RP(p));
				break;
			case 1:	// LD rp[p], (nn)
				RP(p) = getWordViaMemptr(fetchWord());
				break;
			}
			cycles += 20;
			break;
		case 4:	// Negate accumulator
			neg();
			cycles += 8;
			break;
		case 5:	// Return from interrupt
			switch (y) {
			case 1:
				reti();	// RETI
				break;
			default:
				retn();	// RETN
				break;
			}
			cycles += 14;
			break;
		case 6:	// Set interrupt mode
			switch (y) {
			case 0:
			case 4:
				IM() = 0;
				break;
			case 2:
			case 6:
				IM() = 1;
				break;
			case 3:
			case 7:
				IM() = 2;
				break;
			case 1:
			case 5:
				IM() = 0;
			}
			cycles += 8;
			break;
		case 7:	// Assorted ops
			switch (y) {
			case 0:	// LD I,A
				IV() = A();
				cycles += 9;
				break;
			case 1:	// LD R,A
				REFRESH() = A();
				cycles += 9;
				break;
			case 2:	// LD A,I
				A() = IV();
				adjustSign(A());
				adjustZero(A());
				setFlag(PF, IFF2());
				clearFlag(NF | HC);
				cycles += 9;
				break;
			case 3:	// LD A,R
				A() = REFRESH();
				adjustSign(A());
				adjustZero(A());
				clearFlag(HC | NF);
				setFlag(PF, IFF2());
				cycles += 9;
				break;
			case 4:	// RRD
				rrd();
				cycles += 18;
				break;
			case 5:	// RLD
				rld();
				cycles += 18;
				break;
			case 6:	// NOP
				cycles += 4;
				break;
			case 7:	// NOP
				cycles += 4;
				break;
			}
			break;
		}
		break;
	case 2:
		switch (z) {
		case 0:	// LD
			switch (y) {
			case 4:	// LDI
				ldi();
				break;
			case 5:	// LDD
				ldd();
				break;
			case 6:	// LDIR
				ldir();
				break;
			case 7:	// LDDR
				lddr();
				break;
			}
			break;
		case 1:	// CP
			switch (y) {
			case 4:	// CPI
				cpi();
				break;
			case 5:	// CPD
				cpd();
				break;
			case 6:	// CPIR
				cpir();
				break;
			case 7:	// CPDR
				cpdr();
				break;
			}
			break;
		case 2:	// IN
			switch (y) {
			case 4:	// INI
				ini();
				break;
			case 5:	// IND
				ind();
				break;
			case 6:	// INIR
				inir();
				break;
			case 7:	// INDR
				indr();
				break;
			}
			break;
		case 3:	// OUT
			switch (y) {
			case 4:	// OUTI
				outi();
				break;
			case 5:	// OUTD
				outd();
				break;
			case 6:	// OTIR
				otir();
				break;
			case 7:	// OTDR
				otdr();
				break;
			}
			break;
		}
		cycles += 16;
		break;
	}
}

void Z80::executeOther(int x, int y, int z, int p, int q) {
	switch (x) {
	case 0:
		switch (z) {
		case 0:	// Relative jumps and assorted ops
			switch (y) {
			case 0:	// NOP
				cycles += 4;
				break;
			case 1:	// EX AF AF'
				exxAF();
				cycles += 4;
				break;
			case 2:	// DJNZ d
				jrConditional(--B());
				cycles += 8;
				break;
			case 3:	// JR d
				jrConditional(true);
				cycles += 7;
				break;
			default:	// JR cc,d
				jrConditionalFlag(y - 4);
				cycles += 5;
				break;
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0:	// LD rp,nn
				RP(p) = fetchWord();
				cycles += 10;
				break;
			case 1:	// ADD HL,rp
				addViaMemptr(RP(HL_IDX), RP(p));
				cycles += 11;
				break;
			}
			break;
		case 2:	// Indirect loading
			switch (q) {
			case 0:
				switch (p) {
				case 0:	// LD (BC),A
					setViaMemptr(BC().word, A());
					cycles += 7;
					break;
				case 1:	// LD (DE),A
					setViaMemptr(DE().word, A());
					cycles += 7;
					break;
				case 2:	// LD (nn),HL
					setWordViaMemptr(fetchWord(), ALT_HL());
					cycles += 16;
					break;
				case 3: // LD (nn),A
					setViaMemptr(fetchWord(), A());
					cycles += 13;
					break;
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					A() = getViaMemptr(BC().word);
					cycles += 7;
					break;
				case 1:	// LD A,(DE)
					A() = getViaMemptr(DE().word);
					cycles += 7;
					break;
				case 2:	// LD HL,(nn)
					ALT_HL() = getWordViaMemptr(fetchWord());
					cycles += 16;
					break;
				case 3:	// LD A,(nn)
					A() = getViaMemptr(fetchWord());
					cycles += 13;
					break;
				}
				break;
			}
			break;
		case 3:	// 16-bit INC/DEC
			switch (q) {
			case 0:	// INC rp
				++RP(p);
				break;
			case 1:	// DEC rp
				--RP(p);
				break;
			}
			cycles += 6;
			break;
		case 4:	// 8-bit INC
			postIncrement(++R(y));	// INC r
			cycles += 4;
			break;
		case 5:	// 8-bit DEC
			postDecrement(--R(y));	// DEC r
			cycles += 4;
			if (y == 6)
				cycles += 7;
			break;
		case 6: { // 8-bit load immediate
			auto& r = R(y);		// LD r,n
			r = fetchByte();
			cycles += 7;
			if (y == 6)
				cycles += 3;
			break;
		} case 7:	// Assorted operations on accumulator/flags
			switch (y) {
			case 0:
				rlca();
				break;
			case 1:
				rrca();
				break;
			case 2:
				rla();
				break;
			case 3:
				rra();
				break;
			case 4:
				daa();
				break;
			case 5:
				cpl();
				break;
			case 6:
				scf();
				break;
			case 7:
				ccf();
				break;
			}
			cycles += 4;
			break;
		}
		break;
	case 1:	// 8-bit loading
		if (z == 6 && y == 6) { 	// Exception (replaces LD (HL), (HL))
			halt();
		} else {
			bool normal = true;
			if (m_prefixDD || m_prefixFD) {
				if (z == 6) {
					switch (y) {
					case 4:
						H() = R(z);
						normal = false;
						break;
					case 5:
						L() = R(z);
						normal = false;
						break;
					}
				}
				if (y == 6) {
					switch (z) {
					case 4:
						R(y) = H();
						normal = false;
						break;
					case 5:
						R(y) = L();
						normal = false;
						break;
					}
				}
			}
			if (normal)
				R(y) = R(z);
			if ((y == 6) || (z == 6))	// M operations
				cycles += 3;
		}
		cycles += 4;
		break;
	case 2:	// Operate on accumulator and register/memory location
		switch (y) {
		case 0:	// ADD A,r
			A() = add(R(z));
			break;
		case 1:	// ADC A,r
			A() = adc(R(z));
			break;
		case 2:	// SUB r
			A() = sub(R(z));
			break;
		case 3:	// SBC A,r
			A() = sbc(R(z));
			break;
		case 4:	// AND r
			anda(R(z));
			break;
		case 5:	// XOR r
			xora(R(z));
			break;
		case 6:	// OR r
			ora(R(z));
			break;
		case 7:	// CP r
			compare(R(z));
			break;
		}
		cycles += 4;
		if (z == 6)
			cycles += 3;
		break;
	case 3:
		switch (z) {
		case 0:	// Conditional return
			returnConditionalFlag(y);
			cycles += 5;
			break;
		case 1:	// POP & various ops
			switch (q) {
			case 0:	// POP rp2[p]
				RP2(p) = popWord();
				cycles += 10;
				break;
			case 1:
				switch (p) {
				case 0:	// RET
					ret();
					cycles += 10;
					break;
				case 1:	// EXX
					exx();
					cycles += 4;
					break;
				case 2:	// JP HL
					pc = ALT_HL();
					cycles += 4;
					break;
				case 3:	// LD SP,HL
					sp = ALT_HL();
					cycles += 4;
					break;
				}
			}
			break;
		case 2:	// Conditional jump
			jumpConditionalFlag(y);
			cycles += 10;
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0:	// JP nn
				setPcViaMemptr(fetchWord());
				cycles += 10;
				break;
			case 1:	// CB prefix
				m_prefixCB = true;
				if (m_prefixDD || m_prefixFD) {
					m_displacement = fetchByte();
					REFRESH()--;
				}
				execute(fetchByte());
				break;
			case 2: { // OUT (n),A
					auto port = fetchByte();
					m_ports.write(port, A());
					MEMPTR().low = ++port;
					MEMPTR().high = A();
					cycles += 11;
				}
				break;
			case 3: { // IN A,(n)
					auto before = A();
					auto port = fetchByte();
					A() = m_ports.read(port);
					MEMPTR().low = ++port;
					MEMPTR().high = before;
					cycles += 11;
				}
				break;
			case 4:	// EX (SP),HL
				xhtl();
				cycles += 19;
				break;
			case 5:	// EX DE,HL
				std::swap(DE(), HL());
				cycles += 4;
				break;
			case 6:	// DI
				disableInterrupts();
				cycles += 4;
				break;
			case 7:	// EI
				enableInterrupts();
				cycles += 4;
				break;
			}
			break;
		case 4:	// Conditional call: CALL cc[y], nn
			callConditionalFlag(getWord(pc), y);
			cycles += 10;
			break;
		case 5:	// PUSH & various ops
			switch (q) {
			case 0:	// PUSH rp2[p]
				pushWord(RP2(p));
				cycles += 11;
				break;
			case 1:
				switch (p) {
				case 0:	// CALL nn
					callConditional(getWord(pc), true);
					cycles += 17;
					break;
				case 1:	// DD prefix
					m_prefixDD = true;
					execute(fetchByte());
					break;
				case 2:	// ED prefix
					m_prefixED = true;
					execute(fetchByte());
					break;
				case 3:	// FD prefix
					m_prefixFD = true;
					execute(fetchByte());
					break;
				}
			}
			break;
		case 6:	// Operate on accumulator and immediate operand: alu[y] n
			switch (y) {
			case 0:	// ADD A,n
				A() = add(fetchByte());
				break;
			case 1:	// ADC A,n
				A() = adc(fetchByte());
				break;
			case 2:	// SUB n
				A() = sub(fetchByte());
				break;
			case 3:	// SBC A,n
				A() = sbc(fetchByte());
				break;
			case 4:	// AND n
				anda(fetchByte());
				break;
			case 5:	// XOR n
				xora(fetchByte());
				break;
			case 6:	// OR n
				ora(fetchByte());
				break;
			case 7:	// CP n
				compare(fetchByte());
				break;
			}
			cycles += 7;
			break;
		case 7:	// Restart: RST y * 8
			restart(y);
			cycles += 11;
			break;
		}
		break;
	}
}