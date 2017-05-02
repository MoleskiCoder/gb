#pragma once

#include <string>

class Configuration {
public:
	enum MachineMode {
		Unknown,
		ZX81,
		CPM
	};

	Configuration();

	bool isDebugMode() const {
		return m_debugMode;
	}

	void setDebugMode(bool value) {
		m_debugMode = value;
	}

	bool isProfileMode() const {
		return m_profileMode;
	}

	void setProfileMode(bool value) {
		m_profileMode = value;
	}

	bool isDrawGraphics() const {
		return m_drawGraphics;
	}

	void setDrawGraphics(bool value) {
		m_drawGraphics = value;
	}

	bool getVsyncLocked() const {
		return m_vsyncLocked;
	}

	void setVsyncLocked(bool value) {
		m_vsyncLocked = value;
	}

	int getFramesPerSecond() const {
		return m_framesPerSecond;
	}

	void setFramesPerSecond(int value) {
		m_framesPerSecond = value;
	}

	int getCyclesPerFrame() const {
		return m_cyclesPerSecond / getFramesPerSecond();
	}

	int getCyclesPerSecond() const {
		return m_cyclesPerSecond;
	}

	void setCyclesPerSecond(int value) {
		m_cyclesPerSecond = value;
	}

	std::string getRomDirectory() const {
		return m_romDirectory;
	}

	MachineMode getMachineMode() const {
		return m_machineMode;
	}

	void setMachineMode(MachineMode value) {
		m_machineMode = value;
	}

	uint16_t getStartAddress() const {
		return getMachineMode() == CPM ? 0x100 : 0;
	}

private:
	bool m_debugMode;
	bool m_profileMode;
	bool m_drawGraphics;

	bool m_vsyncLocked;
	int m_framesPerSecond;
	int m_cyclesPerSecond;

	std::string m_romDirectory;

	MachineMode m_machineMode;
};
