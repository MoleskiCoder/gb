#include "stdafx.h"
#include "Configuration.h"

Configuration::Configuration()
:	m_debugMode(false),
	m_profileMode(false),
	m_drawGraphics(true),
	m_vsyncLocked(true),
	m_framesPerSecond(60),
	m_cyclesPerSecond(2000000),
	m_romDirectory("roms"),
	m_machineMode(ZX81) {
}