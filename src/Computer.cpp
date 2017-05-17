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
	m_fps(configuration.getFramesPerSecond()),
	m_startTicks(0),
	m_frames(0),
	m_vsync(false) {
}

void Computer::initialise() {

	verifySDLCall(::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC), "Failed to initialise SDL: ");

	m_board.initialise();
	m_board.DrawingLine.connect(std::bind(&Computer::Board_DrawingLine, this, std::placeholders::_1));

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
		auto required = m_configuration.getFramesPerSecond();
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
	m_bitmapTexture = ::SDL_CreateTexture(m_renderer, m_pixelType, SDL_TEXTUREACCESS_STREAMING, Board::RasterWidth, Board::RasterHeight);
	if (m_bitmapTexture == nullptr) {
		throwSDLException("Unable to create bitmap texture");
	}
	m_displayPixels.resize(Board::RasterWidth * Board::RasterHeight);
}

void Computer::runLoop() {

	m_frames = 0UL;
	m_startTicks = ::SDL_GetTicks();

	auto& cpu = m_board.getCPUMutable();

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

		cycles += m_configuration.getCyclesPerFrame();

		cycles -= m_board.runRasterLines();

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

		cycles -= m_board.runVerticalBlankLines();
	}
}

void Computer::handleKeyDown(SDL_Keycode key) {
}

void Computer::handleKeyUp(SDL_Keycode key) {
}

void Computer::drawFrame() {
	
	auto control = m_board.BUS().REG(Bus::LCDC);
	auto on = control & Processor::Bit7;
	if (on) {

		auto windowArea = (control & Processor::Bit6) ? 0x9c00 : 0x9800;
		auto window = (control & Processor::Bit5) != 0;
		auto bgCharacters = (control & Processor::Bit4) ? 0x8000 : 0x8800;
		auto bgArea = (control & Processor::Bit3) ? 0x9c00 : 0x9800;
		auto objBlockHeight = (control & Processor::Bit2) ? 16 : 8;
		auto objDisplay = (control & Processor::Bit1) != 0;
		auto bgDisplay = (control & Processor::Bit0) != 0;

		auto scrollX = m_board.BUS().REG(Bus::SCX);
		auto scrollY = m_board.BUS().REG(Bus::SCY);

		auto paletteRaw = m_board.BUS().REG(Bus::BGP);
		std::array<int, 4> palette;
		palette[0] = paletteRaw & 0b11;
		palette[1] = (paletteRaw & 0b1100) >> 2;
		palette[2] = (paletteRaw & 0b110000) >> 4;
		palette[3] = (paletteRaw & 0b11000000) >> 6;

		auto wx = m_board.BUS().REG(Bus::WX);
		auto wy = m_board.BUS().REG(Bus::WY);

		auto offsetX = window ? wx - 7 : 0;
		auto offsetY = window ? wy : 0;

		std::map<int, CharacterDefinition> definitions;
		
		for (int row = 0; row < Board::BufferCharacterHeight; ++row) {
			for (int column = 0; column < Board::BufferCharacterWidth; ++column) {

				auto address = bgArea + row * Board::BufferCharacterWidth + column;
				auto character = m_board.BUS().peek(address);

				auto definitionPair = definitions.find(character);

				if (definitionPair == definitions.end()) {
					definitions[character] = CharacterDefinition(m_board.BUS(), bgCharacters + 16 * character);
					definitionPair = definitions.find(character);
				}

				auto definition = definitionPair->second;

				for (int cy = 0; cy < 8; ++cy) {
					for (int cx = 0; cx < 8; ++cx) {

						uint8_t x = column * 8 + cx + offsetX - scrollX;
						if (x >= Board::RasterWidth)
							break;

						uint8_t y = row * 8 + cy + offsetY - scrollY;
						if (y >= Board::RasterHeight)
							break;

						auto outputPixel = y * Board::RasterWidth + x;

						auto colour = palette[definition.get()[cy * 8 + cx]];
						m_displayPixels[outputPixel] = m_colours.getColour(colour);
					}
				}
			}
		}
	}

	verifySDLCall(::SDL_UpdateTexture(m_bitmapTexture, NULL, &m_displayPixels[0], Board::RasterWidth * sizeof(Uint32)), "Unable to update texture: ");

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

void Computer::Board_DrawingLine(Board& board) {
}
