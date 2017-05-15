#include "stdafx.h"
#include "Configuration.h"

Configuration::Configuration()
:	m_debugMode(false),
	m_profileMode(false),
	m_drawGraphics(true),
	m_vsyncLocked(true),
	m_framesPerSecond(60),
	m_cyclesPerSecond(4194304),
	m_romDirectory("roms") {
}
