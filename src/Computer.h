#pragma once

#include <stdexcept>
#include <string>
#include <array>

#include <SDL.h>

#include <gb_apu/Gb_Apu.h>
#include <gb_apu/Multi_Buffer.h>
#include <Sound_Queue.h>

#include <Display.h>

#include "Board.h"
#include "ColourPalette.h"

class Configuration;

class Computer {
public:

	static void throwSDLException(std::string failure) {
		throw std::runtime_error(failure + ::SDL_GetError());
	}

	static void verifySDLCall(int returned, std::string failure) {
		if (returned < 0) {
			throwSDLException(failure);
		}
	}

	static void verifyAudioCall(const std::string& context, blargg_err_t error) {
		if (error != nullptr) {
			throw std::runtime_error(context + std::string(": ") + error);
		}
	}

	Computer(const Configuration& configuration);

	void runLoop();
	void initialise();
	
private:
	enum {
		DisplayScale = 2,
	};

	const Configuration& m_configuration;
	mutable Board m_board;
	ColourPalette m_colours;

	SDL_Window* m_window;
	SDL_Renderer* m_renderer;

	SDL_Texture* m_bitmapTexture;
	Uint32 m_pixelType;
	SDL_PixelFormat* m_pixelFormat;

	EightBit::GameBoy::Display m_lcd;

	enum { AudioOutputBufferSize = 4096, AudioSampleRate = 44100 };

	Gb_Apu m_apu;
	Sound_Queue m_audioQueue;
	Stereo_Buffer m_audioMixBuffer;
	std::array<blip_sample_t, AudioOutputBufferSize> m_audioOutputBuffer;
	int m_frameCycles;

	int m_fps;
	Uint32 m_startTicks;
	Uint32 m_frames;
	bool m_vsync;

	void drawFrame();

	void configureBackground() const;
	void createBitmapTexture();

	int getScreenWidth() const {
		return EightBit::GameBoy::Display::RasterWidth * DisplayScale;
	}

	int getScreenHeight() const {
		return EightBit::GameBoy::Display::RasterHeight * DisplayScale;
	}

	void handleKeyDown(SDL_Keycode key);
	void handleKeyUp(SDL_Keycode key);

	static void dumpRendererInformation();
	static void dumpRendererInformation(::SDL_RendererInfo info);

	void initialiseAudio();
	void endAudioframe(int length);

	void Bus_ReadingByte(uint16_t address);
	void Bus_WrittenByte(uint16_t address);

	void Cpu_ExecutedInstruction(const EightBit::GameBoy::LR35902& cpu);
};
