#pragma once

#include <stdexcept>
#include <string>

#include <SDL.h>

#include <Display.h>

#include "Board.h"
#include "ColourPalette.h"
#include "AudioDevice.h"

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
	AudioDevice m_audioDevice;

	SDL_Window* m_window;
	SDL_Renderer* m_renderer;

	SDL_Texture* m_bitmapTexture;
	Uint32 m_pixelType;
	SDL_PixelFormat* m_pixelFormat;

	EightBit::GameBoy::Display m_lcd;

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
};
