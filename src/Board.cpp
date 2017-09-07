#include "stdafx.h"
#include "Board.h"

#include <iostream>

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_cpu(EightBit::GameBoy::LR35902(*this)),
  m_profiler(m_cpu) {
}

void Board::initialise() {

	auto romDirectory = m_configuration.getRomDirectory();

	loadBootRom(romDirectory + "/DMG_ROM.bin");

	//loadGameRom(romDirectory + "/Tetris (World).gb");

	loadGameRom(romDirectory + "/cpu_instrs.gb");				// Passed
	//loadGameRom(romDirectory + "/01-special.gb");				// Passed
	//loadGameRom(romDirectory + "/02-interrupts.gb");			// Passed
	//loadGameRom(romDirectory + "/03-op sp,hl.gb");				// Passed
	//loadGameRom(romDirectory + "/04-op r,imm.gb");				// Passed
	//loadGameRom(romDirectory + "/05-op rp.gb");					// Passed
	//loadGameRom(romDirectory + "/06-ld r,r.gb");				// Passed
	//loadGameRom(romDirectory + "/07-jr,jp,call,ret,rst.gb");	// Passed
	//loadGameRom(romDirectory + "/08-misc instrs.gb");			// Passed
	//loadGameRom(romDirectory + "/09-op r,r.gb");				// Passed
	//loadGameRom(romDirectory + "/10-bit ops.gb");				// Passed
	//loadGameRom(romDirectory + "/11-op a,(hl).gb");				// Passed

	//loadGameRom(romDirectory + "/instr_timing.gb");				// Failed #255
	//loadGameRom(romDirectory + "/interrupt_time.gb");			// Failed

	//loadGameRom(romDirectory + "/opus5.gb");
	//loadGameRom(romDirectory + "/ttt.gb");

	//loadGameRom(romDirectory + "/acceptance/add_sp_e_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/boot_hwio-dmg0.gb");
	//loadGameRom(romDirectory + "/acceptance/boot_hwio-dmgABCXmgb.gb");
	//loadGameRom(romDirectory + "/acceptance/boot_hwio-S.gb");
	//loadGameRom(romDirectory + "/acceptance/boot_regs-dmg0.gb");
	//loadGameRom(romDirectory + "/acceptance/boot_regs-dmgABCX.gb");
	//loadGameRom(romDirectory + "/acceptance/boot_regs-mgb.gb");
	//loadGameRom(romDirectory + "/acceptance/boot_regs-sgb.gb");
	//loadGameRom(romDirectory + "/acceptance/boot_regs-sgb2.gb");
	//loadGameRom(romDirectory + "/acceptance/call_cc_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/call_cc_timing2.gb");
	//loadGameRom(romDirectory + "/acceptance/call_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/call_timing2.gb");
	//loadGameRom(romDirectory + "/acceptance/div_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/di_timing-GS.gb");
	//loadGameRom(romDirectory + "/acceptance/ei_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/halt_ime0_ei.gb");
	//loadGameRom(romDirectory + "/acceptance/halt_ime0_nointr_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/halt_ime1_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/halt_ime1_timing2-GS.gb");
	//loadGameRom(romDirectory + "/acceptance/if_ie_registers.gb");
	//loadGameRom(romDirectory + "/acceptance/intr_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/jp_cc_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/jp_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/ld_hl_sp_e_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/oam_dma_restart.gb");
	//loadGameRom(romDirectory + "/acceptance/oam_dma_start.gb");
	//loadGameRom(romDirectory + "/acceptance/oam_dma_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/pop_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/push_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/rapid_di_ei.gb");
	//loadGameRom(romDirectory + "/acceptance/reti_intr_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/reti_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/ret_cc_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/ret_timing.gb");
	//loadGameRom(romDirectory + "/acceptance/rst_timing.gb");

	//loadGameRom(romDirectory + "/acceptance/timer/div_write.gb");
	//loadGameRom(romDirectory + "/acceptance/timer/rapid_toggle.gb");
	//loadGameRom(romDirectory + "/acceptance/timer/tim00.gb");
	//loadGameRom(romDirectory + "/acceptance/timer/tim00_div_trigger.gb");
	//loadGameRom(romDirectory + "/acceptance/timer/tim01.gb");
	//loadGameRom(romDirectory + "/acceptance/timer/tim01_div_trigger.gb");
	//loadGameRom(romDirectory + "/acceptance/timer/tim10.gb");
	//loadGameRom(romDirectory + "/acceptance/timer/tim10_div_trigger.gb");
	//loadGameRom(romDirectory + "/acceptance/timer/tim11.gb");
	//loadGameRom(romDirectory + "/acceptance/timer/tim11_div_trigger.gb");
	//loadGameRom(romDirectory + "/acceptance/timer/tima_reload.gb");
	//loadGameRom(romDirectory + "/acceptance/timer/tima_write_reloading.gb");
	//loadGameRom(romDirectory + "/acceptance/timer/tma_write_reloading.gb");

	if (m_configuration.isProfileMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Profile, this, std::placeholders::_1));
	}

	if (m_configuration.isDebugMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
	}

	WrittenByte.connect(std::bind(&Board::Bus_WrittenByte, this, std::placeholders::_1));

	reset();
	m_cpu.initialise();
}

void Board::Cpu_ExecutingInstruction_Profile(const EightBit::GameBoy::LR35902& cpu) {
	const auto pc = m_cpu.PC();
	m_profiler.add(pc.word, peek(pc.word));
}

void Board::Cpu_ExecutingInstruction_Debug(const EightBit::GameBoy::LR35902& cpu) {
	if (bootRomDisabled())
		std::cerr
			<< EightBit::GameBoy::Disassembler::state(m_cpu)
			<< " "
			<< m_disassembler.disassemble(m_cpu)
			<< '\n';
}

void Board::Bus_WrittenByte(const EightBit::AddressEventArgs& e) {
	auto address = e.getAddress();
	switch (address) {
	case BASE + SB:
		std::cout << e.getCell();
		break;
	}
}
