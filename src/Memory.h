#pragma once

#include <array>
#include <cstdint>
#include <string>

class Memory {
public:
	static uint8_t lowByte(uint16_t value) { return value & 0xff; }
	static uint8_t highByte(uint16_t value) { return value >> 8; }

	Memory(int addressMask);

	virtual uint16_t& ADDRESS() { return m_address; }
	virtual uint8_t& DATA() { return *m_data; }

	virtual uint8_t& placeDATA(uint8_t value);
	virtual uint8_t& referenceDATA(uint8_t& value);

	virtual uint8_t peek(int address) const;
	virtual uint16_t peekWord(int address) const;

	virtual uint8_t get(int address);
	virtual uint16_t getWord(int address);

	virtual void set(int address, uint8_t value);
	virtual void setWord(int address, uint16_t value);

	virtual uint8_t& reference();
	virtual uint8_t& reference(int address);

	void clear();
	void loadRom(const std::string& path, uint16_t offset);
	void loadRam(const std::string& path, uint16_t offset);

private:
	std::array<uint8_t, 0x10000> m_bus;
	std::array<bool, 0x10000> m_locked;

	int m_addressMask;		// Mirror
	uint8_t m_temporary;	// Used to simulate ROM
	uint16_t m_address;
	uint8_t* m_data;

	int loadMemory(const std::string& path, uint16_t offset);
};
