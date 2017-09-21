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

	loadGameRom(romDirectory + "/games/Tetris (World).gb");
	//loadGameRom(romDirectory + "/games/Dr. Mario (V1.0) (JU) [!].gb");
	//loadGameRom(romDirectory + "/games/Kirby's Dream Land (U) [!].gb");

	//loadGameRom(romDirectory + "/games/opus5.gb");
	//loadGameRom(romDirectory + "/games/ttt.gb");

	//loadGameRom(romDirectory + "/blargg/cpu_instrs.gb");				// Passed
	//loadGameRom(romDirectory + "/blargg/01-special.gb");				// Passed
	//loadGameRom(romDirectory + "/blargg/02-interrupts.gb");			// Passed
	//loadGameRom(romDirectory + "/blargg/03-op sp,hl.gb");				// Passed
	//loadGameRom(romDirectory + "/blargg/04-op r,imm.gb");				// Passed
	//loadGameRom(romDirectory + "/blargg/05-op rp.gb");					// Passed
	//loadGameRom(romDirectory + "/blargg/06-ld r,r.gb");				// Passed
	//loadGameRom(romDirectory + "/blargg/07-jr,jp,call,ret,rst.gb");	// Passed
	//loadGameRom(romDirectory + "/blargg/08-misc instrs.gb");			// Passed
	//loadGameRom(romDirectory + "/blargg/09-op r,r.gb");				// Passed
	//loadGameRom(romDirectory + "/blargg/10-bit ops.gb");				// Passed
	//loadGameRom(romDirectory + "/blargg/11-op a,(hl).gb");				// Passed

	//loadGameRom(romDirectory + "/blargg/instr_timing.gb");				// Failed #255
	//loadGameRom(romDirectory + "/blargg/interrupt_time.gb");			// Failed

	//loadGameRom(romDirectory + "/mooneye/acceptance/add_sp_e_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/boot_hwio-dmg0.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/boot_hwio-dmgABCXmgb.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/boot_hwio-S.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/boot_regs-dmg0.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/boot_regs-dmgABCX.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/boot_regs-mgb.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/boot_regs-sgb.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/boot_regs-sgb2.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/call_cc_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/call_cc_timing2.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/call_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/call_timing2.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/div_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/di_timing-GS.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/ei_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/halt_ime0_ei.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/halt_ime0_nointr_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/halt_ime1_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/halt_ime1_timing2-GS.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/if_ie_registers.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/intr_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/jp_cc_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/jp_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/ld_hl_sp_e_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/oam_dma_restart.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/oam_dma_start.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/oam_dma_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/pop_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/push_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/rapid_di_ei.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/reti_intr_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/reti_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/ret_cc_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/ret_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/rst_timing.gb");

	//loadGameRom(romDirectory + "/mooneye/acceptance/bits/mem_oam.gb");	// Pass
	//loadGameRom(romDirectory + "/mooneye/acceptance/bits/reg_f.gb");	// Pass
	//loadGameRom(romDirectory + "/mooneye/acceptance/bits/unused_hwio-GS.gb");

	//loadGameRom(romDirectory + "/mooneye/acceptance/gpu/hblank_ly_scx_timing-GS.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/gpu/intr_1_2_timing-GS.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/gpu/intr_2_0_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/gpu/intr_2_mode0_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/gpu/intr_2_mode0_timing_sprites.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/gpu/intr_2_mode3_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/gpu/intr_2_oam_ok_timing.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/gpu/lcdon_timing-dmgABCXmgbS.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/gpu/lcdon_write_timing-GS.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/gpu/stat_irq_blocking.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/gpu/vblank_stat_intr-GS.gb");

	//loadGameRom(romDirectory + "/mooneye/acceptance/serial/boot_sclk_align-dmgABCXmgb.gb");

	//loadGameRom(romDirectory + "/mooneye/acceptance/timer/div_write.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/timer/rapid_toggle.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/timer/tim00.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/timer/tim00_div_trigger.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/timer/tim01.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/timer/tim01_div_trigger.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/timer/tim10.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/timer/tim10_div_trigger.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/timer/tim11.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/timer/tim11_div_trigger.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/timer/tima_reload.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/timer/tima_write_reloading.gb");
	//loadGameRom(romDirectory + "/mooneye/acceptance/timer/tma_write_reloading.gb");

	//loadGameRom(romDirectory + "/mooneye/emulator-only/mbc1/multicart_rom_8Mb.gb");

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

void Board::Bus_WrittenByte(const uint16_t address) {
	switch (address) {
	case BASE + SB:
		std::cout << DATA();
		break;
	}
}
