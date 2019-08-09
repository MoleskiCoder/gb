#include "stdafx.h"
#include "Computer.h"
#include "CharacterDefinition.h"

Computer::Computer(const Configuration& configuration)
:	m_configuration(configuration),
	m_board(configuration),
	m_lcd(&m_colours, m_board, m_board.OAMRAM(), m_board.VRAM()),
	m_audioOutputBuffer(AudioOutputBufferSize) {
	verifySDLCall(::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC), "Failed to initialise SDL: ");
}

Computer::~Computer() {
	::SDL_Quit();
}

void Computer::plug(const std::string& path) {
	m_board.plug(path);
}

void Computer::raisePOWER() {

	m_board.raisePOWER();
	m_board.initialise();

	m_window.reset(::SDL_CreateWindow(
		"GameBoy",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		ScreenWidth, ScreenHeight,
		SDL_WINDOW_SHOWN), ::SDL_DestroyWindow);

	if (m_window == nullptr) {
		throwSDLException("Unable to create window: ");
	}

	::SDL_DisplayMode mode;
	verifySDLCall(::SDL_GetWindowDisplayMode(m_window.get(), &mode), "Unable to obtain window information");

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
	m_renderer.reset(::SDL_CreateRenderer(m_window.get(), -1, rendererFlags), ::SDL_DestroyRenderer);
	if (m_renderer == nullptr) {
		throwSDLException("Unable to create renderer: ");
	}

	::SDL_Log("Available renderers:");
	dumpRendererInformation();

	::SDL_RendererInfo info;
	verifySDLCall(::SDL_GetRendererInfo(m_renderer.get(), &info), "Unable to obtain renderer information");
	::SDL_Log("Using renderer:");
	dumpRendererInformation(info);

	if (m_vsync) {
		if ((info.flags & SDL_RENDERER_PRESENTVSYNC) == 0) {
			::SDL_LogWarn(::SDL_LOG_CATEGORY_APPLICATION, "Renderer does not support VSYNC, reverting to timed delay loop.");
			m_vsync = false;
		}
	}

	m_pixelFormat.reset(::SDL_AllocFormat(m_pixelType), ::SDL_FreeFormat);
	if (m_pixelFormat == nullptr) {
		throwSDLException("Unable to allocate pixel format: ");
	}
	m_colours.load(m_pixelFormat.get());

	configureBackground();
	createBitmapTexture();

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

	m_frames = 0UL;
	m_startTicks = ::SDL_GetTicks();

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
}

void Computer::configureBackground() const {
	Uint8 r, g, b;
	::SDL_GetRGB(m_colours.getColour(0), m_pixelFormat.get(), &r, &g, &b);
	verifySDLCall(::SDL_SetRenderDrawColor(m_renderer.get(), r, g, b, SDL_ALPHA_OPAQUE), "Unable to set render draw colour");
}

void Computer::createBitmapTexture() {
	m_bitmapTexture.reset(::SDL_CreateTexture(m_renderer.get(), m_pixelType, SDL_TEXTUREACCESS_STREAMING, EightBit::GameBoy::Display::RasterWidth, EightBit::GameBoy::Display::RasterHeight), ::SDL_DestroyTexture);
	if (m_bitmapTexture == nullptr) {
		throwSDLException("Unable to create bitmap texture");
	}
}

void Computer::run() {

	auto cycles = 0;

	auto graphics = m_configuration.shouldDrawGraphics();

	auto& cpu = m_board.CPU();
	while (cpu.powered()) {

		::SDL_Event e;
		while (::SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				lowerPOWER();
				break;
			case SDL_KEYDOWN:
				handleKeyDown(e.key.keysym.sym);
				break;
			case SDL_KEYUP:
				handleKeyUp(e.key.keysym.sym);
				break;
			}
		}

		cycles += EightBit::GameBoy::Bus::CyclesPerFrame;

		cycles -= m_board.runRasterLines();

		if (graphics) {
			updateTexture();
			displayTexture();
			if (!m_vsync) {
				const auto elapsedTicks = ::SDL_GetTicks() - m_startTicks;
				const auto neededTicks = (++m_frames / (float)m_fps) * 1000.0;
				auto sleepNeeded = (int)(neededTicks - elapsedTicks);
				if (sleepNeeded > 0) {
					::SDL_Delay(sleepNeeded);
				}
			}
		}

		cycles -= m_board.runVerticalBlankLines();
		endAudioframe();
	}
}

void Computer::handleKeyDown(SDL_Keycode key) {
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
	}
}

void Computer::handleKeyUp(SDL_Keycode key) {
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
	}
}

void Computer::updateTexture() {
	verifySDLCall(::SDL_UpdateTexture(m_bitmapTexture.get(), NULL, &(m_lcd.pixels()[0]), EightBit::GameBoy::Display::RasterWidth * sizeof(Uint32)), "Unable to update texture: ");
}

void Computer::displayTexture() {
	verifySDLCall(
		::SDL_RenderCopy(m_renderer.get(), m_bitmapTexture.get(), nullptr, nullptr), 
		"Unable to copy texture to renderer");
	::SDL_RenderPresent(m_renderer.get());
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

void Computer::initialiseAudio() {
	m_apu.set_sample_rate(AudioSampleRate);
	m_audioQueue.start(AudioSampleRate, 2);
}

void Computer::endAudioframe() {

	m_apu.end_frame();

	if (m_apu.samples_avail() >= AudioOutputBufferSize) {
		auto outputBuffer = &m_audioOutputBuffer[0];
		auto count = m_apu.read_samples(m_audioOutputBuffer);
		m_audioQueue.write(outputBuffer, count);
	}
}
