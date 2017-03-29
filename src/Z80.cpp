#include "stdafx.h"
#include "Z80.h"

#include "Memory.h"
#include "Disassembler.h"

Z80::Z80(Memory& memory, InputOutput& ports)
:	m_memory(memory),
	m_ports(ports),
	cycles(0),
	pc(0),
	sp(0),
	a(0),
	f(0),
	a_alt(0),
	f_alt(0),
	iv(0),
	m_interrupt(false),
	m_interruptMode(0),
	m_halted(false) {

	bc.word = de.word = hl.word = 0;
	bc_alt.word = de_alt.word = hl_alt.word = 0;
	ix.word = iy.word = 0;

	installInstructions();
	installInstructionsDD();
	installInstructionsED();
	installInstructionsFD();
}

Z80::Instruction Z80::INS(instruction_t method, AddressingMode mode, std::string disassembly, uint64_t cycles) {
	Z80::Instruction returnValue;
	returnValue.vector = method;
	returnValue.mode = mode;
	returnValue.disassembly = disassembly;
	returnValue.count = cycles;
	return returnValue;
}

Z80::Instruction Z80::UNKNOWN() {
	Z80::Instruction returnValue;
	returnValue.vector = std::bind(&Z80::___, this);
	returnValue.mode = Unknown;
	returnValue.disassembly = "";
	returnValue.count = 0;
	return returnValue;
}

#define BIND(method)	std::bind(&Z80:: method, this)

void Z80::installInstructions() {
	instructions = {
		////	0												1													2													3													4													5												6													7												8												9												A													B												C													D												E												F
		/* 0 */	INS(BIND(nop), Implied, "NOP", 4),				INS(BIND(lxi_b), Absolute, "LD BC,%2$04XH", 10),	INS(BIND(stax_b), Implied, "LD (BC),A", 7),			INS(BIND(inx_b), Implied, "INC BC", 5),				INS(BIND(inr_b), Implied, "INC B", 5),				INS(BIND(dcr_b), Implied, "DEC B", 5),			INS(BIND(mvi_b), Immediate, "LD B,%1$02XH", 7),		INS(BIND(rlc), Implied, "RLCA", 4),				INS(BIND(ex_af_afa), Implied, "EX AF,AF'", 4),	INS(BIND(dad_b), Implied, "ADD HL,BC", 10),		INS(BIND(ldax_b), Implied, "LD A,(BC)", 7),			INS(BIND(dcx_b), Implied, "DEC BC", 5),			INS(BIND(inr_c), Implied, "INC C", 5),				INS(BIND(dcr_c), Implied, "DEC C", 5),			INS(BIND(mvi_c), Immediate, "LD C,%1$02XH", 7),	INS(BIND(rrc), Implied, "RRCA", 4),			//	0
		/* 1 */	INS(BIND(djnz), Relative, "DJNZ %3$04XH", 8),	INS(BIND(lxi_d), Absolute, "LD DE,%2$04XH", 10),	INS(BIND(stax_d), Implied, "LD (DE),A", 7),			INS(BIND(inx_d), Implied, "INC DE", 5),				INS(BIND(inr_d), Implied, "INC D", 5),				INS(BIND(dcr_d), Implied, "DEC D", 5),			INS(BIND(mvi_d), Immediate, "LD D,%1$02XH", 7),		INS(BIND(ral), Implied, "RLA", 4),				UNKNOWN(),										INS(BIND(dad_d), Implied, "ADD HL,DE", 10),		INS(BIND(ldax_d), Implied, "LD A,(DE)", 7),			INS(BIND(dcx_d), Implied, "DEC DE", 5),			INS(BIND(inr_e), Implied, "INC E", 5),				INS(BIND(dcr_e), Implied, "DEC E", 5),			INS(BIND(mvi_e), Immediate, "LD E,%1$02XH", 7),	INS(BIND(rar), Implied, "RRA", 4),			//	1
		/* 2 */	INS(BIND(jrnz), Relative, "JR NZ,%3$04XH", 7),	INS(BIND(lxi_h), Absolute, "LD HL,%2$04XH", 10),	INS(BIND(shld), Absolute, "LD (%2$04XH),HL", 16),	INS(BIND(inx_h), Implied, "INC HL", 5),				INS(BIND(inr_h), Implied, "INC H", 5),				INS(BIND(dcr_h), Implied, "DEC H", 5),			INS(BIND(mvi_h), Immediate, "LD H,%1$02XH",7),		INS(BIND(daa), Implied, "DAA", 4),				INS(BIND(jrz), Relative, "JR Z,%3$04XH", 7),	INS(BIND(dad_h), Implied, "ADD HL,HL", 10),		INS(BIND(lhld), Absolute, "LD HL,(%2$04XH)", 16),	INS(BIND(dcx_h), Implied, "DEC HL", 5),			INS(BIND(inr_l), Implied, "INC L", 5),				INS(BIND(dcr_l), Implied, "DEC L", 5),			INS(BIND(mvi_l), Immediate, "LD L,%1$02XH", 7),	INS(BIND(cma), Implied, "CPL", 4),			//	2
		/* 3 */	INS(BIND(jrnc), Relative, "JR NC,%3$04XH", 7),	INS(BIND(lxi_sp), Absolute, "LD SP,%2$04XH", 10),	INS(BIND(sta), Absolute, "LD (%2$04XH),A ", 13),	INS(BIND(inx_sp), Implied, "INC SP", 5),			INS(BIND(inr_m), Implied, "INC (HL)", 10),			INS(BIND(dcr_m), Implied, "DEC (HL)", 10),		INS(BIND(mvi_m), Immediate, "LD (HL),%1$02XH", 10),	INS(BIND(stc), Implied, "SCF", 4),				INS(BIND(jrc), Relative, "JR C,%3$04XH", 7),	INS(BIND(dad_sp), Implied, "ADD HL,SP", 10),	INS(BIND(lda), Absolute, "LD A,(%2$04XH)", 13),		INS(BIND(dcx_sp), Implied, "DEC SP", 5),		INS(BIND(inr_a), Implied, "INC A", 5),				INS(BIND(dcr_a), Implied, "DEC A", 5),			INS(BIND(mvi_a), Immediate, "LD A,%1$02XH", 7),	INS(BIND(cmc), Implied, "CCF", 4),			//	3

		/* 4 */	INS(BIND(mov_b_b), Implied, "LD B,B", 5),		INS(BIND(mov_b_c), Implied, "LD B,C", 5),			INS(BIND(mov_b_d), Implied, "LD B,D", 5),			INS(BIND(mov_b_e), Implied, "LD B,E", 5),			INS(BIND(mov_b_h), Implied, "LD B,H", 5),			INS(BIND(mov_b_l), Implied, "LD B,L", 5),		INS(BIND(mov_b_m), Implied, "LD B,(HL)", 7),		INS(BIND(mov_b_a), Implied, "LD B,A", 5),		INS(BIND(mov_c_b), Implied, "LD C,B", 5),		INS(BIND(mov_c_c), Implied, "LD C,C", 5),		INS(BIND(mov_c_d), Implied, "LD C,D", 5),			INS(BIND(mov_c_e), Implied, "LD C,E", 5),		INS(BIND(mov_c_h), Implied, "LD C,H", 5),			INS(BIND(mov_c_l), Implied, "LD C,L", 5),		INS(BIND(mov_c_m), Implied, "LD C,(HL)", 7),	INS(BIND(mov_c_a), Implied, "LD C,A", 5),	//	4
		/* 5 */	INS(BIND(mov_d_b), Implied, "LD D,B", 5),		INS(BIND(mov_d_c), Implied, "LD D,C", 5),			INS(BIND(mov_d_d), Implied, "LD D,D", 5),			INS(BIND(mov_d_e), Implied, "LD D,E", 5),			INS(BIND(mov_d_h), Implied, "LD D,H", 5),			INS(BIND(mov_d_l), Implied, "LD D,L", 5),		INS(BIND(mov_d_m), Implied, "LD D,(HL)", 7),		INS(BIND(mov_d_a), Implied, "LD D,A", 5),		INS(BIND(mov_e_b), Implied, "LD E,B", 5),		INS(BIND(mov_e_c), Implied, "LD E,C", 5),		INS(BIND(mov_e_d), Implied, "LD E,D", 5),			INS(BIND(mov_e_e), Implied, "LD E,E", 5),		INS(BIND(mov_e_h), Implied, "LD E,H", 5),			INS(BIND(mov_e_l), Implied, "LD E,L", 5),		INS(BIND(mov_e_m), Implied, "LD E,(HL)", 7),	INS(BIND(mov_e_a), Implied, "LD E,A", 5),	//	5
		/* 6 */	INS(BIND(mov_h_b), Implied, "LD H,B", 5),		INS(BIND(mov_h_c), Implied, "LD H,C", 5),			INS(BIND(mov_h_d), Implied, "LD H,D", 5),			INS(BIND(mov_h_e), Implied, "LD H,E", 5),			INS(BIND(mov_h_h), Implied, "LD H,H", 5),			INS(BIND(mov_h_l), Implied, "LD H,L", 5),		INS(BIND(mov_h_m), Implied, "LD H,(HL)", 7),		INS(BIND(mov_h_a), Implied, "LD H,A", 5),		INS(BIND(mov_l_b), Implied, "LD L,B", 5),		INS(BIND(mov_l_c), Implied, "LD L,C", 5),		INS(BIND(mov_l_d), Implied, "LD L,D", 5),			INS(BIND(mov_l_e), Implied, "LD L,E", 5),		INS(BIND(mov_l_h), Implied, "LD L,H", 5),			INS(BIND(mov_l_l), Implied, "LD L,L", 5),		INS(BIND(mov_l_m), Implied, "LD L,(HL)", 7),	INS(BIND(mov_l_a), Implied, "LD L,A", 5),	//	6
		/* 7 */	INS(BIND(mov_m_b), Implied, "LD (HL),B", 7),	INS(BIND(mov_m_c), Implied, "LD (HL),C", 7),		INS(BIND(mov_m_d), Implied, "LD (HL),D", 7),		INS(BIND(mov_m_e), Implied, "LD (HL),E", 7),		INS(BIND(mov_m_h), Implied, "LD (HL),H", 7),		INS(BIND(mov_m_l), Implied, "LD (HL),L", 7),	INS(BIND(hlt), Implied, "HLT", 7),					INS(BIND(mov_m_a), Implied, "LD (HL),A", 7),	INS(BIND(mov_a_b), Implied, "LD A,B", 5),		INS(BIND(mov_a_c), Implied, "LD A,C", 5),		INS(BIND(mov_a_d), Implied, "LD A,D", 5),			INS(BIND(mov_a_e), Implied, "LD A,E", 5),		INS(BIND(mov_a_h), Implied, "LD A,H", 5),			INS(BIND(mov_a_l), Implied, "LD A,L", 5),		INS(BIND(mov_a_m), Implied, "LD A,(HL)", 7),	INS(BIND(mov_a_a), Implied, "LD A,A", 5),	//	7

		/* 8 */	INS(BIND(add_b), Implied, "ADD A,B", 4),		INS(BIND(add_c), Implied, "ADD A,C", 4),			INS(BIND(add_d), Implied, "ADD A,D", 4),			INS(BIND(add_e), Implied, "ADD A,E", 4),			INS(BIND(add_h), Implied, "ADD A,H", 4),			INS(BIND(add_l), Implied, "ADD A,L", 4),		INS(BIND(add_m), Implied, "ADD A,(HL)", 7),			INS(BIND(add_a), Implied, "ADD A,A", 4),		INS(BIND(adc_b), Implied, "ADC A,B", 4),		INS(BIND(adc_c), Implied, "ADC A,C", 4),		INS(BIND(adc_d), Implied, "ADC A,D", 4),			INS(BIND(adc_e), Implied, "ADC A,E", 4),		INS(BIND(adc_h), Implied, "ADC A,H", 4),			INS(BIND(adc_l), Implied, "ADC A,L", 4),		INS(BIND(adc_m), Implied, "ADC A,(HL)", 4),		INS(BIND(adc_a), Implied, "ADC A,A", 4),	//	8
		/* 9 */	INS(BIND(sub_b), Implied, "SUB B", 4),			INS(BIND(sub_c), Implied, "SUB C", 4),				INS(BIND(sub_d), Implied, "SUB D", 4),				INS(BIND(sub_e), Implied, "SUB E", 4),				INS(BIND(sub_h), Implied, "SUB H", 4),				INS(BIND(sub_l), Implied, "SUB L", 4),			INS(BIND(sub_m), Implied, "SUB (HL)", 7),			INS(BIND(sub_a), Implied, "SUB A", 4),			INS(BIND(sbb_b), Implied, "SBC B", 4),			INS(BIND(sbb_c), Implied, "SBC C", 4),			INS(BIND(sbb_d), Implied, "SBC D", 4),				INS(BIND(sbb_e), Implied, "SBC E", 4),			INS(BIND(sbb_h), Implied, "SBC H", 4),				INS(BIND(sbb_l), Implied, "SBC L", 4),			INS(BIND(sbb_m), Implied, "SBC M", 4),			INS(BIND(sbb_a), Implied, "SBC A", 4),		//	9
		/* A */	INS(BIND(ana_b), Implied, "AND B", 4),			INS(BIND(ana_c), Implied, "AND C", 4),				INS(BIND(ana_d), Implied, "AND D", 4),				INS(BIND(ana_e), Implied, "AND E", 4),				INS(BIND(ana_h), Implied, "AND H", 4),				INS(BIND(ana_l), Implied, "AND L", 4),			INS(BIND(ana_m), Implied, "AND (HL)", 7),			INS(BIND(ana_a), Implied, "AND A", 4),			INS(BIND(xra_b), Implied, "XOR B", 4),			INS(BIND(xra_c), Implied, "XOR C", 4),			INS(BIND(xra_d), Implied, "XOR D", 4),				INS(BIND(xra_e), Implied, "XOR E", 4),			INS(BIND(xra_h), Implied, "XOR H", 4),				INS(BIND(xra_l), Implied, "XOR L", 4),			INS(BIND(xra_m), Implied, "XOR (HL)", 4),		INS(BIND(xra_a), Implied, "XOR A", 4),		//	A
		/* B */	INS(BIND(ora_b), Implied, "OR B", 4),			INS(BIND(ora_c), Implied, "OR C", 4),				INS(BIND(ora_d), Implied, "OR D", 4),				INS(BIND(ora_e), Implied, "OR E", 4),				INS(BIND(ora_h), Implied, "OR H", 4),				INS(BIND(ora_l), Implied, "OR L", 4),			INS(BIND(ora_m), Implied, "OR (HL)", 7),			INS(BIND(ora_a), Implied, "OR A", 4),			INS(BIND(cmp_b), Implied, "CP B", 4),			INS(BIND(cmp_c), Implied, "CP C", 4),			INS(BIND(cmp_d), Implied, "CP D", 4),				INS(BIND(cmp_e), Implied, "CP E", 4),			INS(BIND(cmp_h), Implied, "CP H", 4),				INS(BIND(cmp_l), Implied, "CP L", 4),			INS(BIND(cmp_m), Implied, "CP (HL)", 4),		INS(BIND(cmp_a), Implied, "CP A", 4),		//	B

		/* C */	INS(BIND(rnz), Implied, "RET NZ", 5),			INS(BIND(pop_b), Implied, "POP BC", 10),			INS(BIND(jnz), Absolute, "JP NZ,%2$04XH", 10),		INS(BIND(jmp), Absolute, "JP %2$04XH", 10),			INS(BIND(cnz), Absolute, "CALL NZ,%2$04XH", 11),	INS(BIND(push_b), Implied, "PUSH BC", 11),		INS(BIND(adi), Immediate, "ADD A,%1$02XH", 7),		INS(BIND(rst_0), Implied, "RST 0", 11),			INS(BIND(rz), Implied, "RET Z", 11),			INS(BIND(ret), Implied, "RET", 10),				INS(BIND(jz), Absolute, "JP Z,%2$04XH", 10),		UNKNOWN(),										INS(BIND(cz), Absolute, "CALL Z,%2$04XH", 11),		INS(BIND(call), Absolute, "CALL %2$04XH", 17),	INS(BIND(aci), Immediate, "ADC A,%1$02XH", 7),	INS(BIND(rst_1), Implied, "RST 8", 11),		//	C
		/* D */	INS(BIND(rnc), Implied, "RET NC", 5),			INS(BIND(pop_d), Implied, "POP DE", 10),			INS(BIND(jnc), Absolute, "JP NC,%2$04XH", 10),		INS(BIND(out), Immediate, "OUT (%1$02XH),A", 10),	INS(BIND(cnc), Absolute, "CALL NC,%2$04XH", 11),	INS(BIND(push_d), Implied, "PUSH DE", 11),		INS(BIND(sui), Immediate, "SUB %1$02XH", 7),		INS(BIND(rst_2), Implied, "RST 10H", 11),		INS(BIND(rc), Implied, "RET C", 11),			INS(BIND(exx), Implied, "EXX", 4),				INS(BIND(jc), Absolute, "JP C,%2$04XH", 10),		INS(BIND(in), Immediate, "IN A,(%1$02XH)", 10),	INS(BIND(cc), Absolute, "CALL C,%2$04XH", 11),		UNKNOWN(),										INS(BIND(sbi), Immediate, "SBC %1$02XH", 7),	INS(BIND(rst_3), Implied, "RST 18H", 11),	//	D
		/* E */	INS(BIND(rpo), Implied, "RET PO", 5),			INS(BIND(pop_h), Implied, "POP HL", 10),			INS(BIND(jpo), Absolute, "JP PO,%2$04XH", 10),		INS(BIND(xhtl),	Implied, "EX (SP),HL", 18),			INS(BIND(cpo), Absolute, "CALL PO,%2$04XH", 11),	INS(BIND(push_h), Implied, "PUSH HL", 11),		INS(BIND(ani), Immediate, "AND %1$02XH", 7),		INS(BIND(rst_4), Implied, "RST 20H", 11),		INS(BIND(rpe), Implied, "RET PE", 11),			INS(BIND(pchl), Implied, "JP (HL)", 4),			INS(BIND(jpe), Absolute, "JP PE,%2$04XH", 10),		INS(BIND(xchg), Implied, "EX DE,HL", 4),		INS(BIND(cpe), Absolute, "CALL PE,%2$04XH", 11),	UNKNOWN()/*INS(BIND(ed), Unknown, "", 0)*/,		INS(BIND(xri), Immediate, "XOR %1$02XH", 7),	INS(BIND(rst_5), Implied, "RST 28H", 11),	//	E
		/* F */	INS(BIND(rp), Implied, "RET P", 5),				INS(BIND(pop_psw), Implied, "POP AF", 10),			INS(BIND(jp), Absolute, "JP P,%2$04XH", 10),		INS(BIND(di), Implied, "DI ", 4),					INS(BIND(cp), Absolute, "CALL P,%2$04XH", 11),		INS(BIND(push_psw), Implied, "PUSH AF", 11),	INS(BIND(ori), Immediate, "OR %1$02XH", 7),			INS(BIND(rst_6), Implied, "RST 30H", 11),		INS(BIND(rm), Implied, "RET M", 11),			INS(BIND(sphl), Implied, "LD SP,HL", 5),		INS(BIND(jm), Absolute, "JP M,%2$04XH", 10),		INS(BIND(ei), Implied, "EI", 4),				INS(BIND(cm), Absolute, "CALL M,%2$04XH", 11),		UNKNOWN(),										INS(BIND(cpi), Immediate, "CP %1$02XH", 7),		INS(BIND(rst_7), Implied, "RST 38H", 11),	//	F
	};
}

void Z80::installInstructionsDD() {
	extendedInstructions[0xdd] = {
		////	0												1													2													3													4													5												6													7												8												9												A													B												C													D												E												F
		/* 0 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	0
		/* 1 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	1
		/* 2 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	2
		/* 3 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	3

		/* 4 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	4
		/* 5 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	5
		/* 6 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	6
		/* 7 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	7

		/* 8 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	8
		/* 9 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	9
		/* A */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	A
		/* B */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	B

		/* C */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	C
		/* D */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	D
		/* E */	UNKNOWN(),										INS(BIND(pop_ix), Implied, "POP IX", 14),			UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											INS(BIND(push_ix), Implied, "PUSH IX", 15),		UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										INS(BIND(jp_ix), Implied, "JP (IX)", 4),		UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	E
		/* F */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	F
	};
}

void Z80::installInstructionsED() {
	extendedInstructions[0xed] = {
		////	0												1													2													3													4													5												6													7												8												9												A													B												C													D												E												F
		/* 0 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	0
		/* 1 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	1
		/* 2 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	2
		/* 3 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	3

		/* 4 */	UNKNOWN(),										UNKNOWN(),											INS(BIND(sbc_hl_bc), Implied, "SBC HL,BC", 15),		UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											INS(BIND(ld_i_a), Implied, "LD I,A", 9),		UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	4
		/* 5 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	5
		/* 6 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	6
		/* 7 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	7

		/* 8 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	8
		/* 9 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	9
		/* A */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	A
		/* B */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	B

		/* C */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	C
		/* D */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	D
		/* E */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	E
		/* F */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	F
	};
}

void Z80::installInstructionsFD() {
	extendedInstructions[0xfd] = {
		////	0												1													2													3													4													5												6													7												8												9												A													B												C													D												E												F
		/* 0 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	0
		/* 1 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	1
		/* 2 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	2
		/* 3 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	3

		/* 4 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	4
		/* 5 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	5
		/* 6 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	6
		/* 7 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	7

		/* 8 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	8
		/* 9 */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	9
		/* A */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	A
		/* B */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	B

		/* C */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	C
		/* D */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	D
		/* E */	UNKNOWN(),										INS(BIND(pop_iy), Implied, "POP IY", 14),			UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											INS(BIND(push_iy), Implied, "PUSH IY", 15),		UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										INS(BIND(jp_iy), Implied, "JP (IY)", 4),		UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	E
		/* F */	UNKNOWN(),										UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),											UNKNOWN(),										UNKNOWN(),										UNKNOWN(),									//	F
	};
}

void Z80::reset() {
	pc = 0;
	m_interruptMode = 0;
}

void Z80::initialise() {
	sp = 0;
	bc.word = de.word = hl.word = 0;
	bc_alt.word = de_alt.word = hl_alt.word = 0;
	a = a_alt = 0;
	f = f_alt = 0;
	ix.word = iy.word = 0;
	iv = 0;
	reset();
}

void Z80::step() {

	ExecutingInstruction.fire(CpuEventArgs(*this));

	auto opcode = fetchByte();
	auto extendedIterator = extendedInstructions.find(opcode);
	if (extendedIterator == extendedInstructions.end()) {
		// Normal instruction
		const auto& instruction = instructions[opcode];
		execute(instruction);
	} else {
		// Extended instruction
		opcode = fetchByte();
		const auto& instruction = extendedIterator->second[opcode];
		execute(instruction);
	}
}

//

void Z80::pushWord(uint16_t value) {
	sp -= 2;
	m_memory.setWord(sp, value);
}

uint16_t Z80::popWord() {
	auto value = m_memory.getWord(sp);
	sp += 2;
	return value;
}

//

void Z80::___() {
	auto opcode = m_memory.get(pc - 1);
	auto message = Disassembler::invalid(opcode);
	throw new std::domain_error(message);
}
