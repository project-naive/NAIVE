#pragma once

#ifndef BITSTREAM_H
#define BITSTREAM_H
#include <cstdint>
#include "InStream.h"

class BitStream {
	unsigned char ReadBits(unsigned char bits, uint64_t& dest);
	uint64_t ReadZeros();
	uint64_t ReadOnes();
	bool CheckBuffer();
	uint64_t ReadExpGolomU();
	int64_t ReadExpGolomS();
	//pad the buffering to 8 bytes to best use this stream.
	InStream* stream_source=nullptr;
	//the buffered stream input from callback
	uint64_t self_buffer;
	unsigned char self_buffer_bits=0;
	enum {
		EOF_FLAG = 1,
	};
	unsigned char flags=0;
};

#include "Intrinsics.h"
#include <cassert>
#include <algorithm>
#include "BitStream.inl"

#endif

