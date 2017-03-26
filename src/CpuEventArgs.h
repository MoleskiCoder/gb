#pragma once

class Z80;

class CpuEventArgs {
public:
	CpuEventArgs(const Z80& cpu);

	const Z80& getCpu() const;

private:
	const Z80& m_cpu;
};
