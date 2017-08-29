#include "stdafx.h"
#include "Computer.h"
#include "CharacterDefinition.h"

#include <map>

Computer::Computer(const Configuration& configuration)
:	m_configuration(configuration),
	m_board(configuration),
	m_window(nullptr),
	m_renderer(nullptr),
	m_bitmapTexture(nullptr),
	m_pixelType(SDL_PIXELFORMAT_ARGB8888),
	m_pixelFormat(nullptr),
	m_fps(EightBit::LR35902::framesPerSecond()),
	m_startTicks(0),
	m_frames(0),
	m_vsync(false),
	m_lcd(&m_colours, m_board.BUS()) {
}

void Computer::initialise() {

	verifySDLCall(::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC), "Failed to initialise SDL: ");

	m_board.initialise();

	auto windowWidth = getScreenWidth();
	auto windowHeight = getScreenHeight();

	m_window = ::SDL_CreateWindow(
		"GameBoy",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		windowWidth, windowHeight,
		SDL_WINDOW_SHOWN);

	if (m_window == nullptr) {
		throwSDLException("Unable to create window: ");
	}

	::SDL_DisplayMode mode;
	verifySDLCall(::SDL_GetWindowDisplayMode(m_window, &mode), "Unable to obtain window information");

	m_vsync = m_configuration.getVsyncLocked();
	Uint32 rendererFlags = 0;
	if (m_vsync) {
		auto required = m_fps;
		if (required == mode.refresh_rate) {
			rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
			::SDL_Log("Attempting to use SDL_RENDERER_PRESENTVSYNC");
		} else {
			m_vsync = false;
			::SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Display refresh rate is incompatible with required rate (%d)", required);
		}
	}
	m_renderer = ::SDL_CreateRenderer(m_window, -1, rendererFlags);
	if (m_renderer == nullptr) {
		throwSDLException("Unable to create renderer: ");
	}

	::SDL_Log("Available renderers:");
	dumpRendererInformation();

	::SDL_RendererInfo info;
	verifySDLCall(::SDL_GetRendererInfo(m_renderer, &info), "Unable to obtain renderer information");
	::SDL_Log("Using renderer:");
	dumpRendererInformation(info);

	if (m_vsync) {
		if ((info.flags & SDL_RENDERER_PRESENTVSYNC) == 0) {
			::SDL_LogWarn(::SDL_LOG_CATEGORY_APPLICATION, "Renderer does not support VSYNC, reverting to timed delay loop.");
			m_vsync = false;
		}
	}

	m_pixelFormat = ::SDL_AllocFormat(m_pixelType);
	if (m_pixelFormat == nullptr) {
		throwSDLException("Unable to allocate pixel format: ");
	}
	m_colours.load(m_pixelFormat);

	configureBackground();
	createBitmapTexture();
}

void Computer::configureBackground() const {
	Uint8 r, g, b;
	::SDL_GetRGB(m_colours.getColour(0), m_pixelFormat, &r, &g, &b);
	verifySDLCall(::SDL_SetRenderDrawColor(m_renderer, r, g, b, SDL_ALPHA_OPAQUE), "Unable to set render draw colour");
}

void Computer::createBitmapTexture() {
	m_bitmapTexture = ::SDL_CreateTexture(m_renderer, m_pixelType, SDL_TEXTUREACCESS_STREAMING, EightBit::Display::RasterWidth, EightBit::Display::RasterHeight);
	if (m_bitmapTexture == nullptr) {
		throwSDLException("Unable to create bitmap texture");
	}
	m_lcd.initialise();
}

void Computer::runLoop() {

	m_frames = 0UL;
	m_startTicks = ::SDL_GetTicks();

	auto& cpu = m_board.CPU();

	m_board.powerOn();

	auto cycles = 0;

	while (m_board.powered()) {
		::SDL_Event e;
		while (::SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				m_board.powerOff();
				break;
			case SDL_KEYDOWN:
				handleKeyDown(e.key.keysym.sym);
				break;
			case SDL_KEYUP:
				handleKeyUp(e.key.keysym.sym);
				break;
			}
		}

		cycles += EightBit::LR35902::cyclesPerFrame();

		cycles -= m_board.CPU().runRasterLines();

		if (m_configuration.isDrawGraphics())
			drawFrame();

		::SDL_RenderPresent(m_renderer);
		if (!m_vsync) {
			const auto elapsedTicks = ::SDL_GetTicks() - m_startTicks;
			const auto neededTicks = (++m_frames / (float)m_fps) * 1000.0;
			auto sleepNeeded = (int)(neededTicks - elapsedTicks);
			if (sleepNeeded > 0) {
				::SDL_Delay(sleepNeeded);
			}
		}

		cycles -= m_board.CPU().runVerticalBlankLines();
	}
}

void Computer::handleKeyDown(SDL_Keycode key) {
}

void Computer::handleKeyUp(SDL_Keycode key) {
}

void Computer::drawFrame() {
	
	m_lcd.render();

	verifySDLCall(::SDL_UpdateTexture(m_bitmapTexture, NULL, &(m_lcd.pixels()[0]), EightBit::Display::RasterWidth * sizeof(Uint32)), "Unable to update texture: ");
	verifySDLCall(
		::SDL_RenderCopy(m_renderer, m_bitmapTexture, nullptr, nullptr), 
		"Unable to copy texture to renderer");
}

void Computer::dumpRendererInformation() {
	auto count = ::SDL_GetNumRenderDrivers();
	for (int i = 0; i < count; ++i) {
		::SDL_RendererInfo info;
		verifySDLCall(::SDL_GetRenderDriverInfo(i, &info), "Unable to obtain renderer information");
		dumpRendererInformation(info);
	}
}

void Computer::dumpRendererInformation(::SDL_RendererInfo info) {
	auto name = info.name;
	auto flags = info.flags;
	int software = (flags & SDL_RENDERER_SOFTWARE) != 0;
	int accelerated = (flags & SDL_RENDERER_ACCELERATED) != 0;
	int vsync = (flags & SDL_RENDERER_PRESENTVSYNC) != 0;
	int targetTexture = (flags & SDL_RENDERER_TARGETTEXTURE) != 0;
	::SDL_Log("%s: software=%d, accelerated=%d, vsync=%d, target texture=%d", name, software, accelerated, vsync, targetTexture);
}
