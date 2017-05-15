#include "stdafx.h"
#include "Computer.h"

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
	m_pixels.resize(Board::RasterWidth * Board::RasterHeight);
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

#include <bitset>

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

		auto wx = m_board.BUS().REG(Bus::WX);
		auto wy = m_board.BUS().REG(Bus::WY);

		for (int y = 0; y < Board::RasterHeight; ++y) {
			for (int x = 0; x < Board::RasterWidth; ++x) {
				auto colour = m_colours.getColour(rand() % 4);
					m_pixels[y * Board::RasterWidth + x] = colour;
			}
		}

		//std::cout << "Character tile data::" << std::endl;
		//for (int row = 0; row < Board::BufferCharacterHeight; ++row) {
		//	for (int column = 0; column < Board::BufferCharacterWidth; ++column) {
		//		auto address = bgCharacters + row * Board::BufferCharacterWidth + column;
		//		auto character = m_board.BUS().peek(address);
		//		std::cout << Disassembler::hex(character) << " ";
		//	}
		//	std::cout << std::endl;
		//}
	}

		//auto control = m_board.BUS().REG_LCDC();

		//auto on = control & Processor::Bit7;
		//if (on) {
		//	auto windowTileMapDisplaySelect = control & Processor::Bit6;
		//	auto windowDisplay = control & Processor::Bit5;
		//	auto backgroundAndWindowTileDataSelect = control & Processor::Bit4;
		//	auto backgroundTileMapDisplaySelect = control & Processor::Bit3;
		//	auto spriteSize = control & Processor::Bit2;
		//	auto spriteDisplay = control & Processor::Bit1;
		//	auto backgroundAndWindowDisplay = control & Processor::Bit0;

		//	auto windowTileMapDisplayAddress = windowTileMapDisplaySelect ? 0x9c00 : 0x9800;
		//	auto backgroundAndWindowTileDataAddress = backgroundAndWindowTileDataSelect ? 0x8000 : 0x8800;
		//	auto backgroundTileMapDisplayAddress = backgroundTileMapDisplaySelect ? 0x9c00 : 0x9800;
		//	auto spriteHeight = spriteSize ? 16 : 8;

		//	auto scrollX = m_board.BUS().REG_SCX();
		//	auto scrollY = m_board.BUS().REG_SCY();

		//	auto videoRam = Bus::VideoRam;

		//		//std::cout << std::endl << "**Tiles:" << std::endl;
		//		//for (auto tileRow = 0; tileRow < 32; ++tileRow) {
		//		//	for (auto tileColumn = 0; tileColumn < 32; ++tileColumn) {
		//		//		auto tileAddress = backgroundTileMapDisplayAddress + tileRow * 32 + tileColumn;
		//		//		auto tile = m_board.BUS().peek(tileAddress);
		//		//		std::cout << Disassembler::hex(tile);
		//		//	}
		//		//	std::cout << std::endl;
		//		//}
		//		//std::cout << std::endl << "**" << std::endl;


		//		// backgroundAndWindowTileDataAddress

		//	auto black = m_colours.getColour(ColourPalette::Black);
		//	auto white = m_colours.getColour(ColourPalette::White);

		//		if (backgroundAndWindowTileDataAddress == 0x8000) {

		//			// Assumes scrollX == scrollY == 0
		//			for (auto tileRow = 0; tileRow < 20; ++tileRow) {
		//				for (auto tileColumn = 0; tileColumn < 18; ++tileColumn) {

		//					auto tileAddress = backgroundTileMapDisplayAddress + tileRow * 32 + tileColumn;
		//					auto tile = m_board.BUS().peek(tileAddress);

		//					auto data = backgroundAndWindowTileDataAddress + tile * 8;
		//					for (int y = 0; y < 8; ++y) {
		//						auto value = m_board.BUS().peek(data + y);
		//						for (int bit = 0; bit < 8; ++bit) {
		//							auto mask = 1 << bit;
		//							auto inputPixel = value & mask;
		//							auto colour = inputPixel ? white : black;
		//							auto outputPixel = (tileRow + y) * Board::RasterWidth + tileColumn * 8 + bit;
		//							m_pixels[outputPixel] = colour;

		//						}

		//					}

		//				}
		//			}


		//			// unsigned tile specification!

		//			//backgroundTileMapDisplayAddress

		//		}
			/*
		auto black = m_colours.getColour(ColourPalette::Black);
		auto white = m_colours.getColour(ColourPalette::White);

		auto bytesPerLine = Board::RasterWidth / 8;
		auto begin = videoRam + (scrollY + y) / 8 + scrollX / 8;
		auto x = 0;
		for (auto byte = 0; byte < bytesPerLine; ++byte) {
			auto value = m_board.BUS().peek(begin + byte);
			for (int bit = 0; bit < 8; ++bit) {
				auto mask = 1 << bit;
				auto inputPixel = value & mask;
				auto colour = inputPixel ? white : black;
				auto outputPixel = x++ + y * Board::RasterWidth;
				m_pixels[outputPixel] = colour;

			}
		}
		*/
		//std::cout << "line=" << (int)line << ", SCX=" << (int)scrollX << ",SCY=" << (int)scrollY << std::endl;



		//}










	verifySDLCall(::SDL_UpdateTexture(m_bitmapTexture, NULL, &m_pixels[0], Board::RasterWidth * sizeof(Uint32)), "Unable to update texture: ");

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

	//auto y = board.BUS().REG_LY();
	//if (y == Board::RasterHeight) {

	//	auto control = board.BUS().REG_LCDC();

	//	auto on = control & Processor::Bit7;
	//	if (on) {
	//		auto windowTileMapDisplaySelect = control & Processor::Bit6;
	//		auto windowDisplay = control & Processor::Bit5;
	//		auto backgroundAndWindowTileDataSelect = control & Processor::Bit4;
	//		auto backgroundTileMapDisplaySelect = control & Processor::Bit3;
	//		auto spriteSize = control & Processor::Bit2;
	//		auto spriteDisplay = control & Processor::Bit1;
	//		auto backgroundAndWindowDisplay = control & Processor::Bit0;

	//		auto windowTileMapDisplayAddress = windowTileMapDisplaySelect ? 0x9c00 : 0x9800;
	//		auto backgroundAndWindowTileDataAddress = backgroundAndWindowTileDataSelect ? 0x8000 : 0x8800;
	//		auto backgroundTileMapDisplayAddress = backgroundTileMapDisplaySelect ? 0x9c00 : 0x9800;
	//		auto spriteHeight = spriteSize ? 16 : 8;

	//		auto scrollX = board.BUS().REG_SCX();
	//		auto scrollY = board.BUS().REG_SCY();

	//		auto videoRam = Bus::VideoRam;

	//		if (y == 0) {
	//			std::cout << std::endl << "**Tiles:" << std::endl;
	//			for (auto tileRow = 0; tileRow < 32; ++tileRow) {
	//				for (auto tileColumn = 0; tileColumn < 32; ++tileColumn) {
	//					auto tileAddress = backgroundTileMapDisplayAddress + tileRow * 32 + tileColumn;
	//					auto tile = m_board.BUS().peek(tileAddress);
	//					std::cout << Disassembler::hex(tile);
	//				}
	//				std::cout << std::endl;
	//			}
	//			std::cout << std::endl << "**" << std::endl;


	//			// backgroundAndWindowTileDataAddress

	//			if (backgroundAndWindowTileDataAddress == 0x8000) {
	//				// unsigned tile specification!

	//				//backgroundTileMapDisplayAddress

	//			}
	//		}
	//			/*
	//		auto black = m_colours.getColour(ColourPalette::Black);
	//		auto white = m_colours.getColour(ColourPalette::White);

	//		auto bytesPerLine = Board::RasterWidth / 8;
	//		auto begin = videoRam + (scrollY + y) / 8 + scrollX / 8;
	//		auto x = 0;
	//		for (auto byte = 0; byte < bytesPerLine; ++byte) {
	//			auto value = m_board.BUS().peek(begin + byte);
	//			for (int bit = 0; bit < 8; ++bit) {
	//				auto mask = 1 << bit;
	//				auto inputPixel = value & mask;
	//				auto colour = inputPixel ? white : black;
	//				auto outputPixel = x++ + y * Board::RasterWidth;
	//				m_pixels[outputPixel] = colour;

	//			}
	//		}
	//		*/
	//		//std::cout << "line=" << (int)line << ", SCX=" << (int)scrollX << ",SCY=" << (int)scrollY << std::endl;
		//}

}
