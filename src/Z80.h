#pragma once

#include "Processor.h"

class Z80 : public Processor {
public:
	enum StatusBits {
		SF = Bit7,
		ZF = Bit6,
		YF = Bit5,
		HC = Bit4,
		XF = Bit3,
		PF = Bit2,
		VF = Bit2,
		NF = Bit1,
		CF = Bit0,
	};

	Z80(Memory& memory, InputOutput& ports);

	Signal<Z80> ExecutingInstruction;

	void disableInterrupts();
	void enableInterrupts();

	void interruptMaskable(uint8_t value) { interrupt(true, value); }
	void interruptMaskable() { interruptMaskable(0); }
	void interruptNonMaskable() { interrupt(false, 0); }

	void interrupt(bool maskable, uint8_t value);

	void execute(uint8_t opcode);
	void step();

	bool getM1() const { return m1; }

	virtual uint16_t getWord(int address) const {
		if (getM1())
			throw std::logic_error("M1 cannot be low");
		return Processor::getWord(address);
	}

	// Mutable access to processor!!

	virtual void setWord(int address, uint16_t value) {
		if (getM1())
			throw std::logic_error("M1 cannot be low");
		Processor::setWord(address, value);
	}

	register16_t& AF() {
		return m_accumulatorFlags[m_accumulatorFlagsSet];
	}

	uint8_t& A() { return AF().high; }
	uint8_t& F() { return AF().low; }

	register16_t& BC() {
		return m_registers[m_registerSet][BC_IDX];
	}

	uint8_t& B() { return BC().high; }
	uint8_t& C() { return BC().low; }

	register16_t& DE() {
		return m_registers[m_registerSet][DE_IDX];
	}

	uint8_t& D() { return DE().high; }
	uint8_t& E() { return DE().low; }

	register16_t& HL() {
		return m_registers[m_registerSet][HL_IDX];
	}

	uint8_t& H() { return HL().high; }
	uint8_t& L() { return HL().low; }

	register16_t& IX() { return m_ix; }
	register16_t& IY() { return m_iy; }

	uint8_t& REFRESH() { return m_refresh; }
	uint8_t& IV() { return iv; }
	int& IM() { return m_interruptMode; }
	bool& IFF1() { return m_iff1; }
	bool& IFF2() { return m_iff2; }

	register16_t& MEMPTR() { return m_memptr; }

	bool& M1() { return m1; }

	void exx() {
		m_registerSet ^= 1;
	}

	void exxAF() {
		m_accumulatorFlagsSet = !m_accumulatorFlagsSet;
	}

	virtual void reset();
	virtual void initialise();

private:
	enum { BC_IDX, DE_IDX, HL_IDX };

	std::array<std::array<register16_t, 3>, 2> m_registers;
	int m_registerSet;

	std::array<register16_t, 2> m_accumulatorFlags;
	int m_accumulatorFlagsSet;

	register16_t m_ix;
	register16_t m_iy;

	uint8_t m_refresh;
	uint8_t iv;
	int m_interruptMode;
	bool m_iff1;
	bool m_iff2;

	register16_t m_memptr;

	bool m1;

	bool m_prefixCB;
	bool m_prefixDD;
	bool m_prefixED;
	bool m_prefixFD;

	int8_t m_displacement;

	std::array<bool, 8> m_halfCarryTableAdd = { { false, false, true, false, true, false, true, true } };
	std::array<bool, 8> m_halfCarryTableSub = { { false, true, true, true, false, false, false, true } };

	void fetchExecute() {
		M1() = true;
		execute(fetchByteExecute());
	}

	uint8_t fetchByteExecute() {
		if (!getM1())
			throw std::logic_error("M1 cannot be high");
		return fetchByte();
	}

	uint8_t fetchByteData() {
		if (getM1())
			throw std::logic_error("M1 cannot be low");
		return fetchByte();
	}

	void incrementRefresh() {
		auto incremented = ((REFRESH() & Mask7) + 1) & Mask7;
		REFRESH() = (REFRESH() & Bit7) | incremented;
	}

	void clearFlag(int flag) { F() &= ~flag; }
	void setFlag(int flag) { F() |= flag; }

	void setFlag(int flag, int condition) { setFlag(flag, condition != 0); }
	void setFlag(int flag, uint32_t condition) { setFlag(flag, condition != 0); }
	void setFlag(int flag, bool condition) { condition ? setFlag(flag) : clearFlag(flag); }

	void clearFlag(int flag, int condition) { clearFlag(flag, condition != 0); }
	void clearFlag(int flag, uint32_t condition) { clearFlag(flag, condition != 0); }
	void clearFlag(int flag, bool condition) { condition ? clearFlag(flag) : setFlag(flag); }

	uint8_t& DISPLACED() {
		if (!(m_prefixDD || m_prefixFD))
			throw std::logic_error("Unprefixed indexed displacement requested");
		uint16_t address = (m_prefixDD ? m_ix.word : m_iy.word) + m_displacement;
		MEMPTR().word = address;
		return m_memory.reference(address);
	}

	uint8_t& R(int r, bool followPrefix = true) {
		switch (r) {
		case 0:
			return B();
		case 1:
			return C();
		case 2:
			return D();
		case 3:
			return E();
		case 4:
			if (followPrefix) {
				if (m_prefixDD)
					return m_ix.high;
				if (m_prefixFD)
					return m_iy.high;
			}
			return H();
		case 5:
			if (followPrefix) {
				if (m_prefixDD)
					return m_ix.low;
				if (m_prefixFD)
					return m_iy.low;
			}
			return L();
		case 6:
			if (followPrefix) {
				if (m_prefixDD || m_prefixFD) {
					m_displacement = fetchByteData();
					return DISPLACED();
				}
			}
			return m_memory.reference(HL().word);
		case 7:
			return A();
		}
		throw std::logic_error("Unhandled registry mechanism");
	}

	uint16_t& RP(int rp) {
		switch (rp) {
		case 3:
			return sp;
		case HL_IDX:
			if (m_prefixDD)
				return m_ix.word;
			if (m_prefixFD)
				return m_iy.word;
		default:
			return m_registers[m_registerSet][rp].word;
		}
	}

	uint16_t& ALT_HL() {
		if (m_prefixDD)
			return IX().word;
		else if (m_prefixFD)
			return IY().word;
		return HL().word;
	}

	uint16_t& RP2(int rp) {
		switch (rp) {
		case 3:
			return AF().word;
		case HL_IDX:
			if (m_prefixDD)
				return m_ix.word;
			if (m_prefixFD)
				return m_iy.word;
		default:
			return m_registers[m_registerSet][rp].word;
		}
	}

	uint8_t getViaMemptr(uint16_t address) {
		MEMPTR().word = address + 1;
		return m_memory.get(address);
	}

	void setViaMemptr(uint16_t address, uint8_t value) {
		m_memory.set(address, value);
		MEMPTR().low = Memory::lowByte(++address);
		MEMPTR().high = value;
	}

	uint16_t getWordViaMemptr(uint16_t address) {
		MEMPTR().word = address + 1;
		return getWord(address);
	}

	void setWordViaMemptr(uint16_t address, uint16_t value) {
		setWord(address, value);
		MEMPTR().word = ++address;
	}

	void setPcViaMemptr(uint16_t address) {
		MEMPTR().word = pc = address;
	}

	void addViaMemptr(uint16_t& hl, uint16_t operand) {
		MEMPTR().word = hl + 1;
		hl = add(operand);
	}

	void sbcViaMemptr(uint16_t& hl, uint16_t operand) {
		MEMPTR().word = hl + 1;
		hl = sbc(operand);
	}

	void adcViaMemptr(uint16_t& hl, uint16_t operand) {
		MEMPTR().word = hl + 1;
		hl = adc(operand);
	}

	int buildHalfCarryIndex(uint8_t before, uint8_t value, int calculation) {
		return ((before & 0x88) >> 1) | ((value & 0x88) >> 2) | ((calculation & 0x88) >> 3);
	}

	void adjustHalfCarryAdd(uint8_t before, uint8_t value, int calculation) {
		auto index = buildHalfCarryIndex(before, value, calculation);
		setFlag(HC, m_halfCarryTableAdd[index & 0x7]);
	}

	void adjustHalfCarrySub(uint8_t before, uint8_t value, int calculation) {
		auto index = buildHalfCarryIndex(before, value, calculation);
		setFlag(HC, m_halfCarryTableSub[index & 0x7]);
	}

	void executeCB(int x, int y, int z, int p, int q);
	void executeED(int x, int y, int z, int p, int q);
	void executeOther(int x, int y, int z, int p, int q);

	void adjustSign(uint8_t value);
	void adjustZero(uint8_t value);
	void adjustParity(uint8_t value);
	void adjustSZP(uint8_t value);

	void adjustXYFlags(uint8_t value);

	void postIncrement(uint8_t value);
	void postDecrement(uint8_t value);

	void restart(uint8_t address);

	void jrConditional(int conditional);
	void jrConditionalFlag(int flag);

	void ret();
	void retn();
	void reti();

	void returnConditional(int condition);
	void returnConditionalFlag(int flag);

	void jumpConditional(int condition);
	void jumpConditionalFlag(int flag);

	void call(uint16_t address);
	void callConditional(uint16_t address, int condition);
	void callConditionalFlag(uint16_t address, int flag);

	uint16_t sbc(uint16_t value);
	uint16_t adc(uint16_t value);

	uint16_t add(uint16_t value);

	uint8_t sbc(uint8_t value);
	uint8_t adc(uint8_t value);

	uint8_t sub(uint8_t value);
	uint8_t add(uint8_t value);

	void andr(uint8_t& operand, uint8_t value);

	void anda(uint8_t value);
	void xora(uint8_t value);
	void ora(uint8_t value);
	void compare(uint8_t value);

	void rlca();
	void rrca();
	void rla();
	void rra();

	void rlc(uint8_t& operand);
	void rrc(uint8_t& operand);
	void rl(uint8_t& operand);
	void rr(uint8_t& operand);
	void sla(uint8_t& operand);
	void sra(uint8_t& operand);
	void sll(uint8_t& operand);
	void srl(uint8_t& operand);

	void bit(int n, uint8_t& operand);
	void res(int n, uint8_t& operand);
	void set(int nit, uint8_t& operand);

	void daa();

	void scf();
	void ccf();
	void cpl();

	void xhtl(register16_t& operand);
	void xhtl();

	void cp(uint16_t source);

	void cpi();
	void cpir();

	void cpd();
	void cpdr();

	void blockLoad(uint16_t source, uint16_t destination);

	void ldi();
	void ldir();

	void ldd();
	void lddr();

	void ini();
	void inir();

	void ind();
	void indr();

	void outi();
	void otir();

	void outd();
	void otdr();

	void neg();

	void rrd();
	void rld();

	void readPort(uint8_t& operand, uint8_t port);
};