﻿#pragma once

#include "ByteBuffer.h"

class Packet : public ByteBuffer
{
public:
	Packet() : ByteBuffer()
	{
	}

	Packet(uint8_t opcode) : ByteBuffer(4096)
	{
		append(&opcode, 1);
	}

	Packet(uint8_t opcode, size_t res) : ByteBuffer(res)
	{
		append(&opcode, 1);
	}

	Packet(const Packet &packet) : ByteBuffer(packet)
	{
	}

	uint8_t GetOpcode() const
	{
		return size() == 0 ? 0 : _storage[0];
	}

	//! Clear packet and set opcode all in one mighty blow
	void Initialize(uint8_t opcode)
	{
		clear();
		append(&opcode, 1);
	}
};
