#include "stdafx.h"
#include "LR35902.h"

// based on http://www.z80.info/decoding.htm
// Half carry flag help from https://github.com/oubiwann/z80

LR35902::LR35902(Memory& memory, InputOutput& ports)
: Processor(memory, ports),
  m_refresh(0xff),
  iv(0xff),
  m_interruptMode(0),
  m_iff1(false),
  m_iff2(false),
  m1(false),
  m_prefixCB(false) {
	m_memptr.word = 0;
}

void LR35902::reset() {
	Processor::reset();
	IFF1() = IFF2() = false;
}

void LR35902::initialise() {

	Processor::initialise();

	IM() = 0;

	AF().word = 0xffff;
	BC().word = 0xffff;
	DE().word = 0xffff;
	HL().word = 0xffff;

	REFRESH() = 0xff;
	IV() = 0xff;
	MEMPTR().word = 0;

	m_prefixCB = false;
}

void LR35902::disableInterrupts() {
	IFF1() = IFF2() = false;
}

void LR35902::enableInterrupts() {
	IFF1() = IFF2() = true;
}

void LR35902::interrupt(bool maskable, uint8_t value) {
	if (!maskable || (maskable && IFF1())) {
		if (maskable) {
			disableInterrupts();
			switch (IM()) {
			case 0:
				M1() = true;
				execute(value);
				break;
			case 1:
				restart(7 << 3);
				cycles += 13;
				break;
			case 2:
				pushWord(pc);
				pc = makeWord(value, IV());
				cycles += 19;
				break;
			}
		} else {
			IFF1() = 0;
			restart(0x66);
			cycles += 13;
		}
	}
}

void LR35902::adjustZero(uint8_t value) {
	clearFlag(ZF, value);
}

void LR35902::postIncrement(uint8_t value) {
	adjustZero(value);
	clearFlag(NF);
	clearFlag(HC, lowNibble(value));
}

void LR35902::postDecrement(uint8_t value) {
	adjustZero(value);
	setFlag(NF);
	clearFlag(HC, lowNibble(value + 1));
}

void LR35902::restart(uint8_t address) {
	pushWord(pc);
	setPcViaMemptr(address);
}

void LR35902::jrConditional(int conditional) {
	auto offset = (int8_t)fetchByteData();
	if (conditional) {
		setPcViaMemptr(pc + offset);
		cycles += 5;
	}
}

void LR35902::jrConditionalFlag(int flag) {
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
	case 5:	// PE
	case 6:	// P
	case 7:	// M
		cycles -= 5;
		break;
	}
}

void LR35902::jumpConditional(int conditional) {
	auto address = fetchWord();
	if (conditional) {
		pc = address;
	}
	MEMPTR().word = address;
}

void LR35902::jumpConditionalFlag(int flag) {
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
	case 4:	// GB: LD (FF00 + C),A
		m_memory.set(0xff00 + C(), A());
		cycles -= 2; // Giving 8 cycles
		break;
	case 5:	// GB: LD (nn),A
		m_memory.set(fetchWord(), A());
		cycles += 6; // Giving 16 cycles
		break;
	case 6:	// GB: LD A,(FF00 + C)
		A() = m_memory.get(0xff00 + C());
		cycles -= 2; // 8 cycles
		break;
	case 7:	// GB: LD A,(nn)
		A() = m_memory.get(fetchWord());
		cycles += 6; // Giving 16 cycles
		break;
	}
}

void LR35902::ret() {
	setPcViaMemptr(popWord());
}

void LR35902::retn() {
	ret();
	IFF1() = IFF2();
}

void LR35902::reti() {
	retn();
}

void LR35902::returnConditional(int condition) {
	if (condition) {
		ret();
		cycles += 6;
	}
}

void LR35902::returnConditionalFlag(int flag) {
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
	case 4:	// GB: LD (FF00 + n),A
		m_memory.set(0xff00 + fetchByte(), A());
		cycles += 7; // giving 12 cycles in total
		break;
	case 5: { // GB: ADD SP,dd
			auto before = sp;
			auto value = fetchByte();
			sp += (int8_t)value;
			clearFlag(ZF | NF);
			setFlag(CF, sp & Bit16);
			adjustHalfCarryAdd(Memory::highByte(before), value, Memory::highByte(sp));
		}
		cycles += 11;	// 16 cycles
		break;
	case 6:	// GB: LD A,(FF00 + n)
		A() = m_memory.get(0xff00 + fetchByte());
		cycles += 7;	// 12 cycles
		break;
	case 7:	// GB: LD HL,SP + dd
		HL().word = sp + (int8_t)fetchByte();
		cycles += 7;	// 12 cycles
		break;
	}
}

void LR35902::call(uint16_t address) {
	pushWord(pc + 2);
	pc = address;
}

void LR35902::callConditional(uint16_t address, int condition) {
	if (condition) {
		call(address);
		cycles += 7;
	} else {
		pc += 2;
	}
	MEMPTR().word = address;
}

void LR35902::callConditionalFlag(uint16_t address, int flag) {
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
	case 4:
	case 5:
	case 6:
	case 7:
		cycles -= 10; // removed from GB
		break;
	}
}

uint16_t LR35902::sbc(uint16_t value) {

	auto hl = RP(HL_IDX);

	auto high = Memory::highByte(hl);
	auto highValue = Memory::highByte(value);
	auto applyCarry = F() & CF;

	uint32_t result = (int)hl - (int)value;
	if (applyCarry)
		--result;
	auto highResult = Memory::highByte(result);

	clearFlag(ZF, result);
	adjustHalfCarrySub(high, highValue, highResult);

	setFlag(NF);
	setFlag(CF, result & Bit16);

	return result;
}

uint16_t LR35902::adc(uint16_t value) {

	auto hl = RP(HL_IDX);

	auto high = Memory::highByte(hl);
	auto highValue = Memory::highByte(value);
	auto applyCarry = F() & CF;

	uint32_t result = (int)hl + (int)value;
	if (applyCarry)
		++result;
	auto highResult = Memory::highByte(result);

	clearFlag(ZF, result);
	adjustHalfCarryAdd(high, highValue, highResult);

	clearFlag(NF);
	setFlag(CF, result & Bit16);

	return result;
}

uint16_t LR35902::add(uint16_t value) {

	auto hl = RP(HL_IDX);

	auto high = Memory::highByte(hl);
	auto highValue = Memory::highByte(value);

	uint32_t result = (int)hl + (int)value;

	auto highResult = Memory::highByte(result);

	clearFlag(NF);
	setFlag(CF, result & Bit16);
	adjustHalfCarryAdd(high, highValue, highResult);

	return result;
}

uint8_t LR35902::sbc(uint8_t value) {

	uint16_t result = A() - value;
	if (F() & CF)
		--result;

	clearFlag(ZF, result);
	adjustHalfCarrySub(A(), value, result);

	setFlag(NF);
	setFlag(CF, result & Bit8);

	return (uint8_t)result;
}

uint8_t LR35902::sub(uint8_t value) {

	uint16_t result = A() - value;

	clearFlag(ZF, result);
	adjustHalfCarrySub(A(), value, result);

	setFlag(NF);
	setFlag(CF, result & Bit8);

	return (uint8_t)result;
}

uint8_t LR35902::adc(uint8_t value) {

	uint16_t result = A() + value;
	if (F() & CF)
		++result;

	clearFlag(ZF, result);
	adjustHalfCarryAdd(A(), value, result);

	clearFlag(NF);
	setFlag(CF, result & Bit8);

	return (uint8_t)result;
}

uint8_t LR35902::add(uint8_t value) {

	uint16_t result = A() + value;

	clearFlag(ZF, result);
	adjustHalfCarryAdd(A(), value, result);

	clearFlag(NF);
	setFlag(CF, result & Bit8);

	return (uint8_t)result;
}

//

void LR35902::andr(uint8_t& operand, uint8_t value) {
	setFlag(HC);
	clearFlag(CF | NF);
	operand &= value;
	adjustZero(operand);
}

void LR35902::anda(uint8_t value) {
	andr(A(), value);
}

void LR35902::xora(uint8_t value) {
	clearFlag(HC | CF | NF);
	A() ^= value;
	adjustZero(A());
}

void LR35902::ora(uint8_t value) {
	clearFlag(HC | CF | NF);
	A() |= value;
	adjustZero(A());
}

void LR35902::compare(uint8_t value) {
	sub(value);
}

//

void LR35902::rlc(uint8_t& operand) {
	auto carry = operand & Bit7;
	operand <<= 1;
	setFlag(CF, carry);
	carry ? operand |= Bit0 : operand &= ~Bit0;
	clearFlag(NF | HC);
}

void LR35902::rrc(uint8_t& operand) {
	auto carry = operand & Bit0;
	operand >>= 1;
	carry ? operand |= Bit7 : operand &= ~Bit7;
	setFlag(CF, carry);
	clearFlag(NF | HC);
}

void LR35902::rl(uint8_t& operand) {
	auto oldCarry = F() & CF;
	auto newCarry = operand & Bit7;
	operand <<= 1;
	oldCarry ? operand |= Bit0 : operand &= ~Bit0;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
}

void LR35902::rr(uint8_t& operand) {
	auto oldCarry = F() & CF;
	auto newCarry = operand & Bit0;
	operand >>= 1;
	operand |= oldCarry << 7;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
}

//

void LR35902::sla(uint8_t& operand) {
	auto newCarry = operand & Bit7;
	operand <<= 1;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
}

void LR35902::sra(uint8_t& operand) {
	auto new7 = operand & Bit7;
	auto newCarry = operand & Bit0;
	operand >>= 1;
	operand |= new7;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
}

void LR35902::srl(uint8_t& operand) {
	auto newCarry = operand & Bit0;
	operand >>= 1;
	operand &= ~Bit7;	// clear bit 7
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	setFlag(ZF, operand);
}

//

void LR35902::rlca() {
	rlc(A());
}

void LR35902::rrca() {
	rrc(A());
}

void LR35902::rla() {
	rl(A());
}

void LR35902::rra() {
	rr(A());
}

//

void LR35902::bit(int n, uint8_t& operand) {
	auto carry = F() & CF;
	uint8_t discarded = operand;
	andr(discarded, 1 << n);
	setFlag(CF, carry);
}

void LR35902::res(int n, uint8_t& operand) {
	auto bit = 1 << n;
	operand &= ~bit;
}

void LR35902::set(int n, uint8_t& operand) {
	auto bit = 1 << n;
	operand |= bit;
}

//

void LR35902::daa() {

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

	adjustZero(a);

	A() = a;
}

void LR35902::cpl() {
	A() = ~A();
	setFlag(HC | NF);
}

void LR35902::scf() {
	setFlag(CF);
	clearFlag(HC | NF);
}

void LR35902::ccf() {
	auto carry = F() & CF;
	setFlag(HC, carry);
	clearFlag(CF, carry);
	clearFlag(NF);
}

void LR35902::swap(uint8_t& operand) {
	auto low = lowNibble(operand);
	auto high = highNibble(operand);
	operand = promoteNibble(low) | demoteNibble(high);
	adjustZero(operand);
	clearFlag(NF | HC | CF);
}

void LR35902::neg() {
	auto original = A();
	A() = 0;
	A() = sub(original);
	setFlag(CF, original != 0);
}

void LR35902::rrd() {
	auto accumulator = A();
	auto memory = m_memory.get(HL().word);
	A() = (accumulator & 0xf0) | lowNibble(memory);
	uint8_t updated = promoteNibble(lowNibble(accumulator)) | highNibble(memory);
	m_memory.set(HL().word, updated);
	adjustZero(A());
	clearFlag(NF | HC);
	MEMPTR().word = HL().word + 1;
}

void LR35902::rld() {
	auto accumulator = A();
	auto memory = m_memory.get(HL().word);
	uint8_t updated = lowNibble(accumulator) | promoteNibble(memory);
	A() = (accumulator & 0xf0) | highNibble(memory);
	m_memory.set(HL().word, updated);
	adjustZero(A());
	clearFlag(NF | HC);
	MEMPTR().word = HL().word + 1;
}

void LR35902::readPort(uint8_t& operand, uint8_t port) {
	auto bc = BC().word;
	operand = m_ports.read(port);
	adjustZero(operand);
	clearFlag(HC | NF);
	MEMPTR().word = bc + 1;
}

void LR35902::step() {
	ExecutingInstruction.fire(*this);
	m_prefixCB = false;
	fetchExecute();
}

void LR35902::execute(uint8_t opcode) {

	if (!getM1())
		throw std::logic_error("M1 cannot be high");

	auto x = (opcode & 0b11000000) >> 6;
	auto y = (opcode & 0b111000) >> 3;
	auto z = (opcode & 0b111);

	auto p = (y & 0b110) >> 1;
	auto q = (y & 1);

	auto oldCycles = cycles;

	incrementRefresh();
	M1() = false;

	if (m_prefixCB)
		executeCB(x, y, z, p, q);
	else
		executeOther(x, y, z, p, q);

	auto newCycles = cycles;
	if (newCycles == oldCycles)
		throw std::logic_error("Unhandled opcode");
}

void LR35902::executeCB(int x, int y, int z, int p, int q) {
	switch (x) {
	case 0:	// rot[y] r[z]
		switch (y) {
		case 0:
			rlc(R(z));
			break;
		case 1:
			rrc(R(z));
			break;
		case 2:
			rl(R(z));
			break;
		case 3:
			rr(R(z));
			break;
		case 4:
			sla(R(z));
			break;
		case 5:
			sra(R(z));
			break;
		case 6:
			swap(R(z));
			break;
		case 7:
			srl(R(z));
			break;
		}
		adjustZero(R(z));
		cycles += 8;
		if (z == 6)
			cycles += 7;
		break;
	case 1: { // BIT y, r[z]
			auto operand = R(z);
			bit(y, operand);
			cycles += 8;
			if (z == 6)
				cycles += 4;
		}
		break;
	case 2:	// RES y, r[z]
		res(y, R(z));
		cycles += 8;
		if (z == 6)
			cycles += 7;
		break;
	case 3:	// SET y, r[z]
		set(y, R(z));
		cycles += 8;
		if (z == 6)
			cycles += 7;
		break;
	}
}

void LR35902::executeOther(int x, int y, int z, int p, int q) {
	switch (x) {
	case 0:
		switch (z) {
		case 0:	// Relative jumps and assorted ops
			switch (y) {
			case 0:	// NOP
				cycles += 4;
				break;
			case 1:	// GB: LD (nn),SP
				m_memory.setWord(fetchWord(), sp);
				cycles += 20;
				break;
			case 2:	// GB: STOP
				stop();
				cycles += 4;
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
				case 2:	// GB: LDI (HL),A
					m_memory.set(HL().word++, A());
					cycles += 8;
					break;
				case 3: // GB: LDD (HL),A
					m_memory.set(HL().word--, A());
					cycles += 8;
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
				case 2:	// GB: LDI A,(HL)
					A() = m_memory.get(HL().word++);
					cycles += 8;
					break;
				case 3:	// GB: LDD A,(HL)
					A() = m_memory.get(HL().word--);
					cycles += 8;
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
			r = fetchByteData();
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
				case 1:	// GB: RETI
					reti();
					cycles += 8;
					break;
				case 2:	// JP HL
					pc = HL().word;
					cycles += 4;
					break;
				case 3:	// LD SP,HL
					sp = HL().word;
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
				fetchExecute();
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
				}
			}
			break;
		case 6:	// Operate on accumulator and immediate operand: alu[y] n
			switch (y) {
			case 0:	// ADD A,n
				A() = add(fetchByteData());
				break;
			case 1:	// ADC A,n
				A() = adc(fetchByteData());
				break;
			case 2:	// SUB n
				A() = sub(fetchByteData());
				break;
			case 3:	// SBC A,n
				A() = sbc(fetchByteData());
				break;
			case 4:	// AND n
				anda(fetchByteData());
				break;
			case 5:	// XOR n
				xora(fetchByteData());
				break;
			case 6:	// OR n
				ora(fetchByteData());
				break;
			case 7:	// CP n
				compare(fetchByteData());
				break;
			}
			cycles += 7;
			break;
		case 7:	// Restart: RST y * 8
			restart(y << 3);
			cycles += 11;
			break;
		}
		break;
	}
}