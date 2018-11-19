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
#include "Configuration.h"

class Computer final {
public:
	Computer(const Configuration& configuration);
	~Computer();

	void run();
	void plug(const std::string& path);

	void powerOn();
	void powerOff();
	
private:
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

	enum {
		DisplayScale = 2,
		ScreenWidth = EightBit::GameBoy::Display::RasterWidth * DisplayScale,
		ScreenHeight = EightBit::GameBoy::Display::RasterHeight * DisplayScale,
	};

	const Configuration& m_configuration;
	mutable Board m_board;
	ColourPalette m_colours;

	std::shared_ptr<SDL_Window> m_window;
	std::shared_ptr<SDL_Renderer> m_renderer;
	std::shared_ptr<SDL_PixelFormat> m_pixelFormat;
	std::shared_ptr<SDL_Texture> m_bitmapTexture;
	Uint32 m_pixelType = SDL_PIXELFORMAT_ARGB8888;

	EightBit::GameBoy::Display m_lcd;

	enum { AudioOutputBufferSize = 4096, AudioSampleRate = 44100 };

	Gb_Apu m_apu;
	Sound_Queue m_audioQueue;
	Stereo_Buffer m_audioMixBuffer;
	std::array<blip_sample_t, AudioOutputBufferSize> m_audioOutputBuffer;
	int m_frameCycles = 0;

	int m_fps = EightBit::GameBoy::Bus::FramesPerSecond;
	Uint32 m_startTicks = 0;
	Uint32 m_frames = 0;
	bool m_vsync = false;

	void drawFrame();

	void configureBackground() const;
	void createBitmapTexture();

	void handleKeyDown(SDL_Keycode key);
	void handleKeyUp(SDL_Keycode key);

	static void dumpRendererInformation();
	static void dumpRendererInformation(::SDL_RendererInfo info);

	void initialiseAudio();
	void endAudioframe(int length);
};
