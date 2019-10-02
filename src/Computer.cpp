#include "stdafx.h"
#include "Computer.h"
#include "CharacterDefinition.h"

Computer::Computer(const Configuration& configuration)
:	m_configuration(configuration),
	m_board(configuration),
	m_lcd(&m_colours, m_board, m_board.OAMRAM(), m_board.VRAM()),
	m_audioOutputBuffer(AudioOutputBufferSize) {
}

void Computer::plug(const std::string& path) {
	m_board.plug(path);
}

void Computer::raisePOWER() {

	Gaming::Game::raisePOWER();

	m_board.raisePOWER();
	m_board.initialise();

	m_colours.load(pixelFormat().get());

	m_board.ReadByte.connect([this](const EightBit::EventArgs&) {
		const auto address = m_board.ADDRESS().word;
		if (address >= Gb_Apu::start_addr && address <= Gb_Apu::end_addr) {
			auto value = m_apu.read_register(address);
			m_board.DATA() = value;
		}
	});

	m_board.WrittenByte.connect([this] (const EightBit::EventArgs&) {
		const auto address = m_board.ADDRESS().word;
		if (address > Gb_Apu::start_addr && address <= Gb_Apu::end_addr)
			m_apu.write_register(address, m_board.DATA());
	});

	initialiseAudio();

	m_board.IO().DisplayStatusModeUpdated.connect([this] (const int mode) {
		switch (mode & EightBit::Processor::Mask2) {
		case EightBit::GameBoy::IoRegisters::LcdStatusMode::HBlank:
			break;
		case EightBit::GameBoy::IoRegisters::LcdStatusMode::VBlank:
			break;
		case EightBit::GameBoy::IoRegisters::LcdStatusMode::SearchingOamRam:
			m_lcd.loadObjectAttributes();
			break;
		case EightBit::GameBoy::IoRegisters::LcdStatusMode::TransferringDataToLcd:
			m_lcd.render();
			break;
		}
	});
}

void Computer::lowerPOWER() {
	m_board.lowerPOWER();
	Gaming::Game::lowerPOWER();
}

void Computer::runRasterLines() {
	m_board.runRasterLines();
}

void Computer::runVerticalBlank() {
	m_board.runVerticalBlankLines();
	endAudioframe();
}

const uint32_t* Computer::pixels() const noexcept {
	return m_lcd.pixels().data();
}

bool Computer::handleKeyDown(SDL_Keycode key) {
	const auto handled = Gaming::Game::handleKeyDown(key);
	if (handled)
		return true;
	switch (key) {
	case SDLK_UP:
		m_board.IO().pressUp();
		break;
	case SDLK_DOWN:
		m_board.IO().pressDown();
		break;
	case SDLK_LEFT:
		m_board.IO().pressLeft();
		break;
	case SDLK_RIGHT:
		m_board.IO().pressRight();
		break;
	case SDLK_z:
		m_board.IO().pressB();
		break;
	case SDLK_x:
		m_board.IO().pressA();
		break;
	case SDLK_BACKSPACE:
		m_board.IO().pressSelect();
		break;
	case SDLK_RETURN:
		m_board.IO().pressStart();
		break;
	default:
		return false;
	}
	return true;
}

bool Computer::handleKeyUp(SDL_Keycode key) {
	const auto handled = Gaming::Game::handleKeyUp(key);
	if (handled)
		return true;
	switch (key) {
	case SDLK_UP:
		m_board.IO().releaseUp();
		break;
	case SDLK_DOWN:
		m_board.IO().releaseDown();
		break;
	case SDLK_LEFT:
		m_board.IO().releaseLeft();
		break;
	case SDLK_RIGHT:
		m_board.IO().releaseRight();
		break;
	case SDLK_z:
		m_board.IO().releaseB();
		break;
	case SDLK_x:
		m_board.IO().releaseA();
		break;
	case SDLK_BACKSPACE:
		m_board.IO().releaseSelect();
		break;
	case SDLK_RETURN:
		m_board.IO().releaseStart();
		break;
	default:
		return false;
	}
	return true;
}

void Computer::initialiseAudio() {
	m_apu.set_sample_rate(AudioSampleRate);
	m_audioQueue.start(AudioSampleRate, 2);
}

void Computer::endAudioframe() {

	m_apu.end_frame();

	if (m_apu.samples_avail() >= AudioOutputBufferSize) {
		auto outputBuffer = &m_audioOutputBuffer[0];
		const auto count = m_apu.read_samples(m_audioOutputBuffer);
		m_audioQueue.write(outputBuffer, count);
	}
}
