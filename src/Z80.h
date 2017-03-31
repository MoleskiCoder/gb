#pragma once

#include <cstdint>
#include <array>
#include <functional>
#include <map>

#include "Memory.h"
#include "StatusFlags.h"
#include "InputOutput.h"
#include "Signal.h"
#include "CpuEventArgs.h"

class Z80 {
public:

	typedef union {
		struct {
#ifdef HOST_LITTLE_ENDIAN
			uint8_t low;
			uint8_t high;
#endif
#ifdef HOST_BIG_ENDIAN
			uint8_t high;
			uint8_t low;
#endif
		};
		uint16_t word;
	} register16_t;

	typedef std::function<void()> instruction_t;

	enum AddressingMode {
		Unknown,
		Implied,	// zero bytes
		Immediate,	// single byte
		Relative,	// single byte, relative to pc
		Absolute	// two bytes, little endian
	};

	struct Instruction {
		instruction_t vector = nullptr;
		AddressingMode mode = Unknown;
		std::string disassembly;
		uint64_t count = 0;
	};

	Z80(Memory& memory, InputOutput& ports);

	Signal<CpuEventArgs> ExecutingInstruction;

	const std::array<Instruction, 0x100>& getInstructions() const { return instructions; }
	const std::array<Instruction, 0x100>& getExtendedInstructions(uint8_t surrogate) const { return extendedInstructions.find(surrogate)->second; }
	bool hasExtendedInstructions(uint8_t surrogate) const { return extendedInstructions.find(surrogate) != extendedInstructions.cend(); }

	const Memory& getMemory() const { return m_memory; }

	uint16_t getProgramCounter() const { return pc; }
	void setProgramCounter(uint16_t value) { pc = value; }

	uint16_t getStackPointer() const { return sp; }

	uint8_t getA() const { return a; }
	StatusFlags getF() const { return f; }

	uint8_t getA_Alt() const { return a_alt; }
	StatusFlags getF_Alt() const { return f_alt; }

	const register16_t& getBC() const { return bc; }
	const register16_t& getDE() const { return de; }
	const register16_t& getHL() const { return hl; }

	const register16_t& getBC_Alt() const { return bc_alt; }
	const register16_t& getDE_Alt() const { return de_alt; }
	const register16_t& getHL_Alt() const { return hl_alt; }

	const register16_t& getIX() const { return ix; }
	const register16_t& getIY() const { return iy; }

	bool isInterruptable() const {
		return m_interrupt;
	}

	void disableInterrupts() { m_interrupt = false; }
	void enableInterrupts() { m_interrupt = true; }

	void interrupt(uint8_t value) {
		if (isInterruptable()) {
			disableInterrupts();
			switch (m_interruptMode) {
			case 0:
				execute(value);
				break;
			case 1:
				restart(7);
				cycles += 13;
				break;
			case 2:
				pushWord(pc);
				pc = Memory::makeWord(value, iv);
				cycles += 19;
				break;
			}
		}
	}

	bool isHalted() const { return m_halted; }
	void halt() { m_halted = true; }

	void initialise();

	void reset();
	void step();

private:
	std::array<Instruction, 0x100> instructions;
	std::map<uint8_t, std::array<Instruction, 0x100>> extendedInstructions;

	std::array<bool, 8> m_halfCarryTableAdd = { { false, false, true, false, true, false, true, true } };
	std::array<bool, 8> m_halfCarryTableSub = { { false, true, true, true, false, false, false, true } };

	Memory& m_memory;
	InputOutput& m_ports;

	uint64_t cycles;

	uint16_t pc;
	uint16_t sp;

	uint8_t a;
	uint8_t a_alt;
	StatusFlags f;
	StatusFlags f_alt;

	uint8_t iv;
	int m_interruptMode;
	bool m_iff1;
	bool m_iff2;

	register16_t bc;
	register16_t bc_alt;

	register16_t de;
	register16_t de_alt;

	register16_t hl;
	register16_t hl_alt;

	register16_t ix;
	register16_t iy;

	bool m_interrupt;
	bool m_halted;

	void execute(uint8_t opcode);

	void execute(const Instruction& instruction) {
		instruction.vector();
		cycles += instruction.count;
	}

	void adjustSign(uint8_t value) { f.SF = ((value & 0x80) != 0); }
	void adjustZero(uint8_t value) { f.ZF = (value == 0); }

	void adjustParity(uint8_t value) {
		static const uint8_t lookup[0x10] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
		auto set = (lookup[value >> 4] + lookup[value & 0xF]);
		f.PF = (set % 2) == 0;
	}

	void adjustSZP(uint8_t value) {
		adjustSign(value);
		adjustZero(value);
		adjustParity(value);
	}

	int buildHalfCarryIndex(uint8_t value, int calculation) {
		return ((a & 0x88) >> 1) | ((value & 0x88) >> 2) | ((calculation & 0x88) >> 3);
	}

	void adjustHalfCarryAdd(uint8_t value, int calculation) {
		auto index = buildHalfCarryIndex(value, calculation);
		f.HF = m_halfCarryTableAdd[index & 0x7];
	}

	void adjustAuxiliaryCarrySub(uint8_t value, int calculation) {
		auto index = buildHalfCarryIndex(value, calculation);
		f.HF = !m_halfCarryTableSub[index & 0x7];
	}

	void postIncrement(uint8_t value) {
		adjustSZP(value);
		f.HF = (value & 0x0f) == 0;
	}

	void postDecrement(uint8_t value) {
		adjustSZP(value);
		f.HF = (value & 0x0f) != 0xf;
	}

	void pushWord(uint16_t value);
	uint16_t popWord();

	uint8_t fetchByte() {
		return m_memory.get(pc++);
	}

	uint16_t fetchWord() {
		auto value = m_memory.getWord(pc);
		pc += 2;
		return value;
	}

	static Instruction INS(instruction_t method, AddressingMode mode, std::string disassembly, uint64_t cycles);
	Instruction UNKNOWN();

	void installInstructions();
	void installInstructionsDD();
	void installInstructionsED();
	void installInstructionsFD();

	//

	void compare(uint8_t value) {
		uint16_t subtraction = a - value;
		adjustSZP((uint8_t)subtraction);
		adjustAuxiliaryCarrySub(value, subtraction);
		f.CF = subtraction > 0xff;
	}

	void callAddress(uint16_t address) {
		pushWord(pc + 2);
		pc = address;
	}

	void restart(uint8_t position) {
		uint16_t address = position << 3;
		pushWord(pc);
		pc = address;
	}

	void callConditional(int condition) {
		if (condition) {
			call();
			cycles += 6;
		} else {
			pc += 2;
		}
	}

	void returnConditional(int condition) {
		if (condition) {
			ret();
			cycles += 6;
		}
	}

	void jmpConditional(int conditional) {
		auto destination = fetchWord();
		if (conditional)
			pc = destination;
	}

	void jrConditional(int conditional) {
		auto offset = (int8_t)fetchByte();
		if (conditional) {
			pc += offset;
			cycles += 5;
		}
	}

	void anda(uint8_t value) {
		f.HF = (((a | value) & 0x8) != 0);
		f.CF = false;
		adjustSZP(a &= value);
	}

	void ora(uint8_t value) {
		f.HF = f.CF = false;
		adjustSZP(a |= value);
	}

	void xra(uint8_t value) {
		f.HF = f.CF = false;
		adjustSZP(a ^= value);
	}

	void add(uint8_t value) {
		uint16_t sum = a + value;
		a = Memory::lowByte(sum);
		f.CF = sum > 0xff;
		adjustSZP(a);
		adjustHalfCarryAdd(value, sum);
	}

	void adc(uint8_t value) {
		add(value + f.CF);
	}

	void dad(uint16_t value) {
		uint32_t sum = hl.word + value;
		f.CF = sum > 0xffff;
		hl.word = (uint16_t)sum;
	}

	void subByte(uint8_t value) {
		uint16_t difference = a - value;
		a = Memory::lowByte(difference);
		f.CF = difference > 0xff;
		adjustSZP(a);
		adjustAuxiliaryCarrySub(value, difference);
	}

	void subWord(uint16_t value) {
		auto subtraction = hl.word - value - f.CF;
		hl.word = subtraction;
		adjustSign(hl.high);
		f.ZF = (hl.word == 0);
		f.PF = subtraction > 0x10000;
	}

	void sbb(uint8_t value) {
		subByte(value + f.CF);
	}

	void mov_m_r(uint8_t value) {
		m_memory.set(hl.word, value);
	}

	uint8_t mov_r_m() {
		return m_memory.get(hl.word);
	}

	//

	uint8_t loadByteAddressRelative(uint16_t address) {
		auto offset = (int8_t)fetchByte();
		return m_memory.get(address + offset);
	}

	//

	void ___();

	// Move, load, and store

	void mov_a_a() { }
	void mov_a_b() { a = bc.high; }
	void mov_a_c() { a = bc.low; }
	void mov_a_d() { a = de.high; }
	void mov_a_e() { a = de.low; }
	void mov_a_h() { a = hl.high; }
	void mov_a_l() { a = hl.low; }

	void mov_b_a() { bc.high = a; }
	void mov_b_b() { }
	void mov_b_c() { bc.high = bc.low; }
	void mov_b_d() { bc.high = de.high; }
	void mov_b_e() { bc.high = de.low; }
	void mov_b_h() { bc.high = hl.high; }
	void mov_b_l() { bc.high = hl.low; }

	void mov_c_a() { bc.low = a; }
	void mov_c_b() { bc.low = bc.high; }
	void mov_c_c() { }
	void mov_c_d() { bc.low = de.high; }
	void mov_c_e() { bc.low = de.low; }
	void mov_c_h() { bc.low = hl.high; }
	void mov_c_l() { bc.low = hl.low; }

	void mov_d_a() { de.high = a; }
	void mov_d_b() { de.high = bc.high; }
	void mov_d_c() { de.high = bc.low; }
	void mov_d_d() { }
	void mov_d_e() { de.high = de.low; }
	void mov_d_h() { de.high = hl.high; }
	void mov_d_l() { de.high = hl.low; }

	void mov_e_a() { de.low = a; }
	void mov_e_b() { de.low = bc.high; }
	void mov_e_c() { de.low = bc.low; }
	void mov_e_d() { de.low = de.high; }
	void mov_e_e() { }
	void mov_e_h() { de.low = hl.high; }
	void mov_e_l() { de.low = hl.low; }

	void mov_h_a() { hl.high = a; }
	void mov_h_b() { hl.high = bc.high; }
	void mov_h_c() { hl.high = bc.low; }
	void mov_h_d() { hl.high = de.high; }
	void mov_h_e() { hl.high = de.low; }
	void mov_h_h() { }
	void mov_h_l() { hl.high = hl.low; }

	void mov_l_a() { hl.low = a; }
	void mov_l_b() { hl.low = bc.high; }
	void mov_l_c() { hl.low = bc.low; }
	void mov_l_d() { hl.low = de.high; }
	void mov_l_e() { hl.low = de.low; }
	void mov_l_h() { hl.low = hl.high; }
	void mov_l_l() { }

	void mov_m_a() { mov_m_r(a); }
	void mov_m_b() { mov_m_r(bc.high); }
	void mov_m_c() { mov_m_r(bc.low); }
	void mov_m_d() { mov_m_r(de.high); }
	void mov_m_e() { mov_m_r(de.low); }
	void mov_m_h() { mov_m_r(hl.high); }
	void mov_m_l() { mov_m_r(hl.low); }

	void mov_a_m() { a = mov_r_m(); }
	void mov_b_m() { bc.high = mov_r_m(); }
	void mov_c_m() { bc.low = mov_r_m(); }
	void mov_d_m() { de.high = mov_r_m(); }
	void mov_e_m() { de.low = mov_r_m(); }
	void mov_h_m() { hl.high = mov_r_m(); }
	void mov_l_m() { hl.low = mov_r_m(); }

	void mov_i_a() { iv = a; }

	void mvi_a() { a = fetchByte(); }
	void mvi_b() { bc.high = fetchByte(); }
	void mvi_c() { bc.low = fetchByte(); }
	void mvi_d() { de.high = fetchByte(); }
	void mvi_e() { de.low = fetchByte(); }
	void mvi_h() { hl.high = fetchByte(); }
	void mvi_l() { hl.low = fetchByte(); }

	void mvi_m() {
		auto data = fetchByte();
		m_memory.set(hl.word, data);
	}

	void mov_a_ix_i() { a = loadByteAddressRelative(ix.word); }
	void mov_a_iy_i() { a = loadByteAddressRelative(iy.word); }

	void lxi_b() { bc.word = fetchWord(); }
	void lxi_d() { de.word = fetchWord(); }
	void lxi_h() { hl.word = fetchWord(); }

	void lxi_ix() { ix.word = fetchWord(); }
	void lxi_iy() { iy.word = fetchWord(); }
	
	void stax_b() { m_memory.set(bc.word, a); }
	void stax_d() { m_memory.set(de.word, a); }

	void ldax_b() { a = m_memory.get(bc.word); }
	void ldax_d() { a = m_memory.get(de.word); }

	void sta() {
		auto destination = fetchWord();
		m_memory.set(destination, a);
	}

	void lda() {
		auto source = fetchWord();
		a = m_memory.get(source);
	}

	void shld() {
		auto destination = fetchWord();
		m_memory.setWord(destination, hl.word);
	}

	void lhld() {
		auto source = fetchWord();
		hl.word = m_memory.getWord(source);
	}

	void xchg() {
		std::swap(de, hl);
	}

	void mov_m_sp() {
		auto address = fetchWord();
		m_memory.setWord(address, sp);
	}

	void mov_sp_m() {
		auto address = fetchWord();
		sp = m_memory.getWord(address);
	}

	// stack ops

	void push_b() { pushWord(bc.word); }
	void push_d() { pushWord(de.word); }
	void push_h() { pushWord(hl.word); }

	void push_ix() { pushWord(ix.word); }
	void push_iy() { pushWord(iy.word); }

	void push_psw() {
		auto pair = Memory::makeWord(f, a);
		pushWord(pair);
	}

	void pop_b() { bc.word = popWord(); }
	void pop_d() { de.word = popWord(); }
	void pop_h() { hl.word = popWord(); }

	void pop_ix() { ix.word = popWord(); }
	void pop_iy() { iy.word = popWord(); }

	void pop_psw() {
		auto af = popWord();
		a = Memory::highByte(af);
		f = Memory::lowByte(af);
	}

	void xhtl() {
		auto tos = m_memory.getWord(sp);
		m_memory.setWord(sp, hl.word);
		hl.word = tos;
	}

	void sphl() {
		sp = hl.word;
	}

	void lxi_sp() {
		sp = fetchWord();
	}

	void inx_sp() { ++sp; }
	void dcx_sp() { --sp; }

	// jump

	void jmp() { jmpConditional(true); }

	void jc() { jmpConditional(f.CF); }
	void jnc() { jmpConditional(!f.CF); }

	void jz() { jmpConditional(f.ZF); }
	void jnz() { jmpConditional(!f.ZF); }

	void jpe() { jmpConditional(f.PF); }
	void jpo() { jmpConditional(!f.PF); }

	void jm() { jmpConditional(f.SF); }
	void jp() { jmpConditional(!f.SF); }

	void pchl() { pc = hl.word; }

	void jp_ix() { pc = ix.word; }
	void jp_iy() { pc = iy.word; }

	void djnz() { jrConditional(--bc.high); }

	void jrz() { jrConditional(f.ZF); }
	void jrnz() { jrConditional(!f.ZF); }

	void jrc() { jrConditional(f.CF); }
	void jrnc() { jrConditional(!f.CF); }

	// call

	void call() {
		auto destination = m_memory.getWord(pc);
		callAddress(destination);
	}

	void cc() { callConditional(f.CF); }
	void cnc() { callConditional(!f.CF); }

	void cpe() { callConditional(f.PF); }
	void cpo() { callConditional(!f.PF); }

	void cz() { callConditional(f.ZF); }
	void cnz() { callConditional(!f.ZF); }

	void cm() { callConditional(f.SF); }
	void cp() { callConditional(!f.SF); }

	// return

	void ret() {
		pc = popWord();
	}

	void rc() { returnConditional(f.CF); }
	void rnc() { returnConditional(!f.CF); }

	void rz() { returnConditional(f.ZF); }
	void rnz() { returnConditional(!f.ZF); }

	void rpe() { returnConditional(f.PF); }
	void rpo() { returnConditional(!f.PF); }

	void rm() { returnConditional(f.SF); }
	void rp() { returnConditional(!f.SF); }

	// restart

	void rst_0() { restart(0); }
	void rst_1() { restart(1); }
	void rst_2() { restart(2); }
	void rst_3() { restart(3); }
	void rst_4() { restart(4); }
	void rst_5() { restart(5); }
	void rst_6() { restart(6); }
	void rst_7() { restart(7); }

	// increment and decrement

	void inr_a() { postIncrement(++a); }
	void inr_b() { postIncrement(++bc.high); }
	void inr_c() { postIncrement(++bc.low); }
	void inr_d() { postIncrement(++de.high); }
	void inr_e() { postIncrement(++de.low); }
	void inr_h() { postIncrement(++hl.high); }
	void inr_l() { postIncrement(++hl.low); }

	void inr_m() {
		auto value = m_memory.get(hl.word);
		postIncrement(++value);
		m_memory.set(hl.word, value);
	}

	void dcr_a() { postDecrement(--a); }
	void dcr_b() { postDecrement(--bc.high); }
	void dcr_c() { postDecrement(--bc.low); }
	void dcr_d() { postDecrement(--de.high); }
	void dcr_e() { postDecrement(--de.low); }
	void dcr_h() { postDecrement(--hl.high); }
	void dcr_l() { postDecrement(--hl.low); }

	void dcr_m() {
		auto value = m_memory.get(hl.word);
		postDecrement(--value);
		m_memory.set(hl.word, value);
	}

	void inx_b() { ++bc.word; }
	void inx_d() { ++de.word; }
	void inx_h() { ++hl.word; }

	void inx_ix() { ++ix.word; }
	void inx_iy() { ++iy.word; }

	void dcx_b() { --bc.word; }
	void dcx_d() { --de.word; }
	void dcx_h() { --hl.word; }

	// add

	void add_a() { add(a); }
	void add_b() { add(bc.high); }
	void add_c() { add(bc.low); }
	void add_d() { add(de.high); }
	void add_e() { add(de.low); }
	void add_h() { add(hl.high); }
	void add_l() { add(hl.low); }

	void add_m() {
		auto value = m_memory.get(hl.word);
		add(value);
	}

	void adi() { add(fetchByte()); }

	void adc_a() { adc(a); }
	void adc_b() { adc(bc.high); }
	void adc_c() { adc(bc.low); }
	void adc_d() { adc(de.high); }
	void adc_e() { adc(de.low); }
	void adc_h() { adc(hl.high); }
	void adc_l() { adc(hl.low); }

	void adc_m() {
		auto value = m_memory.get(hl.word);
		adc(value);
	}

	void aci() { adc(fetchByte()); }

	void dad_b() { dad(bc.word); }
	void dad_d() { dad(de.word); }
	void dad_h() { dad(hl.word); }
	void dad_sp() { dad(sp); }

	// subtract

	void sub_a() { subByte(a); }
	void sub_b() { subByte(bc.high); }
	void sub_c() { subByte(bc.low); }
	void sub_d() { subByte(de.high); }
	void sub_e() { subByte(de.low); }
	void sub_h() { subByte(hl.high); }
	void sub_l() { subByte(hl.low); }

	void sub_m() {
		auto value = m_memory.get(hl.word);
		subByte(value);
	}

	void sbb_a() { sbb(a); }
	void sbb_b() { sbb(bc.high); }
	void sbb_c() { sbb(bc.low); }
	void sbb_d() { sbb(de.high); }
	void sbb_e() { sbb(de.low); }
	void sbb_h() { sbb(hl.high); }
	void sbb_l() { sbb(hl.low); }

	void sbb_m() {
		auto value = m_memory.get(hl.word);
		sbb(value);
	}

	void sbi() {
		auto value = fetchByte();
		sbb(value);
	}

	void sui() {
		auto value = fetchByte();
		subByte(value);
	}

	void sbc_hl_bc() { subWord(bc.word); }
	void sbc_hl_de() { subWord(de.word); }
	void sbc_hl_hl() { subWord(hl.word); }
	void sbc_hl_sp() { subWord(sp); }

	// logical

	void ana_a() { anda(a); }
	void ana_b() { anda(bc.high); }
	void ana_c() { anda(bc.low); }
	void ana_d() { anda(de.high); }
	void ana_e() { anda(de.low); }
	void ana_h() { anda(hl.high); }
	void ana_l() { anda(hl.low); }

	void ana_m() {
		auto value = m_memory.get(hl.word);
		anda(value);
	}

	void ani() { anda(fetchByte()); }

	void xra_a() { xra(a); }
	void xra_b() { xra(bc.high); }
	void xra_c() { xra(bc.low); }
	void xra_d() { xra(de.high); }
	void xra_e() { xra(de.low); }
	void xra_h() { xra(hl.high); }
	void xra_l() { xra(hl.low); }

	void xra_m() {
		auto value = m_memory.get(hl.word);
		xra(value);
	}

	void xri() { xra(fetchByte()); }

	void ora_a() { ora(a); }
	void ora_b() { ora(bc.high); }
	void ora_c() { ora(bc.low); }
	void ora_d() { ora(de.high); }
	void ora_e() { ora(de.low); }
	void ora_h() { ora(hl.high); }
	void ora_l() { ora(hl.low); }

	void ora_m() {
		auto value = m_memory.get(hl.word);
		ora(value);
	}

	void ori() { ora(fetchByte()); }

	void cmp_a() { compare(a); }
	void cmp_b() { compare(bc.high); }
	void cmp_c() { compare(bc.low); }
	void cmp_d() { compare(de.high); }
	void cmp_e() { compare(de.low); }
	void cmp_h() { compare(hl.high); }
	void cmp_l() { compare(hl.low); }

	void cmp_m() {
		auto value = m_memory.get(hl.word);
		compare(value);
	}

	void cpi() { compare(fetchByte()); }

	// rotate

	void rlc() {
		auto carry = a & 0x80;
		a <<= 1;
		a |= carry >> 7;
		f.CF = carry != 0;
	}

	void rrc() {
		auto carry = a & 1;
		a >>= 1;
		a |= carry << 7;
		f.CF = carry != 0;
	}

	void ral() {
		auto carry = a & 0x80;
		a <<= 1;
		a |= (uint8_t)f.CF;
		f.CF = carry != 0;
	}

	void rar() {
		auto carry = a & 1;
		a >>= 1;
		a |= f.CF << 7;
		f.CF = carry != 0;
	}

	// specials

	void cma() {
		a ^= 0xff;
	}

	void stc() {
		f.CF = true;
	}

	void cmc() {
		f.CF = !f.CF;
	}

	void daa() {
		auto carry = f.CF;
		uint8_t addition = 0;
		if (f.HF || (a & 0xf) > 9) {
			addition = 0x6;
		}
		if (f.CF || (a >> 4) > 9 || ((a >> 4) >= 9 && (a & 0xf) > 9)) {
			addition |= 0x60;
			carry = true;
		}
		add(addition);
		f.CF = carry;
	}

	// input/output

	void out() {
		auto port = fetchByte();
		m_ports.write(port, a);
	}

	void in() {
		auto port = fetchByte();
		a = m_ports.read(port);
	}

	void in_b_c() { bc.high = m_ports.read(bc.low); }
	void in_d_c() { de.high = m_ports.read(bc.low); }
	void in_h_c() { hl.high = m_ports.read(bc.low); }

	void out_c_b() { m_ports.write(bc.low, bc.high); }
	void out_c_d() { m_ports.write(bc.low, de.high); }
	void out_c_h() { m_ports.write(bc.low, hl.high); }

	// control

	void ei() { enableInterrupts(); }
	void di() { disableInterrupts(); }

	void nop() {}

	void hlt() {
		m_halted = true;
	}

	// alternate register set

	void ex_af_afa() {
		std::swap(a, a_alt);
		std::swap(f, f_alt);
	}

	void exx() {
		std::swap(bc, bc_alt);
		std::swap(de, de_alt);
		std::swap(hl, hl_alt);
	}

	// block operations

	void ldi() {
		m_memory.set(de.word++, m_memory.get(hl.word++));
		--bc.word;
		f.NF = f.HF = false;
		f.PF = bc.word != 0;
	}

	void ldir() {
		ldi();
		if (f.PF) {		// See LDI
			cycles += 5;
			pc -= 2;
		}
	}

	// interrupts

	void im_0() {
		m_interruptMode = 0;
	}

	void im_1() {
		m_interruptMode = 1;
	}

	void im_2() {
		m_interruptMode = 2;
	}
};
