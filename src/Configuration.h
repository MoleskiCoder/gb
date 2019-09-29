#pragma once

#include <string>

class Configuration final {
public:
	Configuration() = default;

	bool isDebugMode() const noexcept {
		return m_debugMode;
	}

	void setDebugMode(bool value) noexcept {
		m_debugMode = value;
	}

	bool shouldDrawGraphics() const noexcept {
		return m_drawGraphics;
	}

	void setDrawGraphics(bool value) noexcept {
		m_drawGraphics = value;
	}

	bool getVsyncLocked() const noexcept {
		return m_vsyncLocked;
	}

	void setVsyncLocked(bool value) noexcept {
		m_vsyncLocked = value;
	}

	std::string getRomDirectory() const noexcept {
		return m_romDirectory;
	}

private:
	bool m_debugMode = false;
	bool m_drawGraphics = true;
	bool m_vsyncLocked = true;
	std::string m_romDirectory = "roms";
};
