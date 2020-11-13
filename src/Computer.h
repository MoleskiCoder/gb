#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <Game.h>

#include <Basic_Gb_Apu.h>
#include <Sound_Queue.h>

#include <Display.h>

#include "Board.h"
#include "ColourPalette.h"
#include "Configuration.h"

class Computer final : public Gaming::Game {
public:
	Computer(const Configuration& configuration);

	void plug(std::string path);

	void raisePOWER() final;
	void lowerPOWER() final;
	
protected:
	float fps() const noexcept final { return EightBit::GameBoy::Bus::FramesPerSecond; }
	bool useVsync() const noexcept final { return m_configuration.getVsyncLocked(); }

	int rasterWidth() const noexcept final { return EightBit::GameBoy::Display::RasterWidth; }
	int rasterHeight() const noexcept final { return EightBit::GameBoy::Display::RasterHeight; }
	int displayScale() const noexcept final { return 2; }

	std::string title() const noexcept final { return "GameBoy"; }

	const uint32_t* pixels() const noexcept final;

	bool handleKeyDown(SDL_Keycode key) final;
	bool handleKeyUp(SDL_Keycode key) final;

	void runRasterLines() final;
	void runVerticalBlank() final;

private:
	const Configuration& m_configuration;
	mutable Board m_board;
	ColourPalette m_colours;

	EightBit::GameBoy::Display m_lcd;

	enum { AudioOutputBufferSize = 4096, AudioSampleRate = 44100 };

	Basic_Gb_Apu m_apu;
	Sound_Queue m_audioQueue;
	std::vector<int16_t> m_audioOutputBuffer;

	void initialiseAudio();
	void endAudioframe();
};
