#include "stdafx.h"

#include "Configuration.h"
#include "Computer.h"

int main(int, char*[])
{
	Configuration configuration;

#ifdef _DEBUG
	configuration.setDebugMode(true);
	configuration.setProfileMode(true);
#endif

	Computer computer(configuration);

	//computer.plug("jmla09.gb");

	computer.plug("games/Tetris (World).gb");
	//computer.plug("games/Dr. Mario (V1.0) (JU) [!].gb");
	//computer.plug("games/Kirby's Dream Land (U) [!].gb");

	//computer.plug("games/opus5.gb");
	//computer.plug("games/ttt.gb");

	//computer.plug("blargg/cpu_instrs.gb");				// Passed
	//computer.plug("blargg/01-special.gb");				// Passed
	//computer.plug("blargg/02-interrupts.gb");			// Passed
	//computer.plug("blargg/03-op sp,hl.gb");				// Passed
	//computer.plug("blargg/04-op r,imm.gb");				// Passed
	//computer.plug("blargg/05-op rp.gb");					// Passed
	//computer.plug("blargg/06-ld r,r.gb");				// Passed
	//computer.plug("blargg/07-jr,jp,call,ret,rst.gb");	// Passed
	//computer.plug("blargg/08-misc instrs.gb");			// Passed
	//computer.plug("blargg/09-op r,r.gb");				// Passed
	//computer.plug("blargg/10-bit ops.gb");				// Passed
	//computer.plug("blargg/11-op a,(hl).gb");				// Passed

	//computer.plug("blargg/instr_timing.gb");				// Failed #255
	//computer.plug("blargg/interrupt_time.gb");			// Failed

	//computer.plug("mooneye/acceptance/add_sp_e_timing.gb");
	//computer.plug("mooneye/acceptance/boot_hwio-dmg0.gb");
	//computer.plug("mooneye/acceptance/boot_hwio-dmgABCXmgb.gb");
	//computer.plug("mooneye/acceptance/boot_hwio-S.gb");
	//computer.plug("mooneye/acceptance/boot_regs-dmg0.gb");
	//computer.plug("mooneye/acceptance/boot_regs-dmgABCX.gb");
	//computer.plug("mooneye/acceptance/boot_regs-mgb.gb");
	//computer.plug("mooneye/acceptance/boot_regs-sgb.gb");
	//computer.plug("mooneye/acceptance/boot_regs-sgb2.gb");
	//computer.plug("mooneye/acceptance/call_cc_timing.gb");
	//computer.plug("mooneye/acceptance/call_cc_timing2.gb");
	//computer.plug("mooneye/acceptance/call_timing.gb");
	//computer.plug("mooneye/acceptance/call_timing2.gb");
	//computer.plug("mooneye/acceptance/div_timing.gb");
	//computer.plug("mooneye/acceptance/di_timing-GS.gb");
	//computer.plug("mooneye/acceptance/ei_timing.gb");
	//computer.plug("mooneye/acceptance/halt_ime0_ei.gb");
	//computer.plug("/mooneye/acceptance/halt_ime0_nointr_timing.gb");
	//computer.plug("mooneye/acceptance/halt_ime1_timing.gb");
	//computer.plug("mooneye/acceptance/halt_ime1_timing2-GS.gb");
	//computer.plug("mooneye/acceptance/if_ie_registers.gb");
	//computer.plug("mooneye/acceptance/intr_timing.gb");
	//computer.plug("mooneye/acceptance/jp_cc_timing.gb");
	//computer.plug("mooneye/acceptance/jp_timing.gb");
	//computer.plug("mooneye/acceptance/ld_hl_sp_e_timing.gb");
	//computer.plug("mooneye/acceptance/oam_dma_restart.gb");
	//computer.plug("mooneye/acceptance/oam_dma_start.gb");
	//computer.plug("mooneye/acceptance/oam_dma_timing.gb");
	//computer.plug("mooneye/acceptance/pop_timing.gb");
	//computer.plug("mooneye/acceptance/push_timing.gb");
	//computer.plug("mooneye/acceptance/rapid_di_ei.gb");
	//computer.plug("mooneye/acceptance/reti_intr_timing.gb");
	//computer.plug("mooneye/acceptance/reti_timing.gb");
	//computer.plug("mooneye/acceptance/ret_cc_timing.gb");
	//computer.plug("mooneye/acceptance/ret_timing.gb");
	//computer.plug("mooneye/acceptance/rst_timing.gb");

	//computer.plug("mooneye/acceptance/bits/mem_oam.gb");	// Pass
	//computer.plug("mooneye/acceptance/bits/reg_f.gb");	// Pass
	//computer.plug("mooneye/acceptance/bits/unused_hwio-GS.gb");

	//computer.plug("mooneye/acceptance/gpu/hblank_ly_scx_timing-GS.gb");
	//computer.plug("mooneye/acceptance/gpu/intr_1_2_timing-GS.gb");
	//computer.plug("mooneye/acceptance/gpu/intr_2_0_timing.gb");
	//computer.plug("mooneye/acceptance/gpu/intr_2_mode0_timing.gb");
	//computer.plug("mooneye/acceptance/gpu/intr_2_mode0_timing_sprites.gb");
	//computer.plug("mooneye/acceptance/gpu/intr_2_mode3_timing.gb");
	//computer.plug("mooneye/acceptance/gpu/intr_2_oam_ok_timing.gb");
	//computer.plug("mooneye/acceptance/gpu/lcdon_timing-dmgABCXmgbS.gb");
	//computer.plug("mooneye/acceptance/gpu/lcdon_write_timing-GS.gb");
	//computer.plug("mooneye/acceptance/gpu/stat_irq_blocking.gb");
	//computer.plug("mooneye/acceptance/gpu/vblank_stat_intr-GS.gb");

	//computer.plug("mooneye/acceptance/serial/boot_sclk_align-dmgABCXmgb.gb");

	//computer.plug("mooneye/acceptance/timer/div_write.gb");
	//computer.plug("mooneye/acceptance/timer/rapid_toggle.gb");
	//computer.plug("mooneye/acceptance/timer/tim00.gb");
	//computer.plug("mooneye/acceptance/timer/tim00_div_trigger.gb");
	//computer.plug("mooneye/acceptance/timer/tim01.gb");
	//computer.plug("mooneye/acceptance/timer/tim01_div_trigger.gb");
	//computer.plug("mooneye/acceptance/timer/tim10.gb");
	//computer.plug("mooneye/acceptance/timer/tim10_div_trigger.gb");
	//computer.plug("mooneye/acceptance/timer/tim11.gb");
	//computer.plug("mooneye/acceptance/timer/tim11_div_trigger.gb");
	//computer.plug("mooneye/acceptance/timer/tima_reload.gb");
	//computer.plug("mooneye/acceptance/timer/tima_write_reloading.gb");
	//computer.plug("mooneye/acceptance/timer/tma_write_reloading.gb");

	//computer.plug("mooneye/emulator-only/mbc1/multicart_rom_8Mb.gb");

	computer.powerOn();
	try {
		computer.run();
	} catch (const std::exception& error) {
		::SDL_LogError(::SDL_LOG_CATEGORY_APPLICATION, "%s", error.what());
		return 2;
	}

	return 0;
}