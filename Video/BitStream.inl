#include "BitStream.h"

inline unsigned char BitStream::ReadBits(unsigned char bits, uint64_t& dest)
{
	assert(stream_source);
	dest = 0;
	if (bits > 64) bits = 64;
	unsigned char left = bits;
	uint64_t rtn = 0;
	while (left && !((flags & EOF_FLAG)&&!self_buffer_bits)) {
		CheckBuffer();
		if (left <= self_buffer_bits) {
			self_buffer_bits -= left;
			rtn |= ((self_buffer >> (self_buffer_bits))& (0xFFFFFFFFFFFFFFFFUi64 >> (64 - left)));
			left = 0;
		}
		else {
			left -= self_buffer_bits;
			rtn |= ((self_buffer & (0xFFFFFFFFFFFFFFFFUi64 >> (64 - self_buffer_bits))) << left);
			self_buffer_bits = 0;
		}
	}
	dest = rtn;
	return bits - left;
}

inline uint64_t BitStream::ReadZeros()
{
	uint64_t rtn = 0;
	uint64_t bits=0;
	do {
		CheckBuffer();
		bits = (self_buffer & (0xFFFFFFFFFFFFFFFFUi64 >> (64 - self_buffer_bits)));
		if(!bits) {
			rtn += self_buffer_bits;
			self_buffer_bits = 0;
		}
		else {
			uint32_t cur;
			bscan64f(bits, cur);
			rtn += cur-(64- self_buffer_bits);
		}
	} while (!bits && !(flags & EOF_FLAG));
	return rtn;
}

inline uint64_t BitStream::ReadOnes()
{
	uint64_t rtn = 0;
	uint64_t bits = 0;
	uint64_t mask;
	do {
		CheckBuffer();
		mask = (0xFFFFFFFFFFFFFFFFUi64 >> (64 - self_buffer_bits));
		bits = (self_buffer & mask);
		if (bits == mask) {
			rtn += self_buffer_bits;
			self_buffer_bits = 0;
		}
		else {
			uint32_t cur;
			bscan64f(((bits ^ 0xFFFFFFFFFFFFFFFFUi64) & mask), cur);
			cur = cur - (64 - self_buffer_bits);;
			rtn += cur;
			self_buffer_bits -= cur;
		}
	} while (bits == mask && !(flags & EOF_FLAG));
	return rtn;
}

inline bool BitStream::CheckBuffer()
{
	if (!self_buffer_bits && !flags) {
		self_buffer_bits = stream_source->readBytes(&self_buffer, 8);
		//little endian system
		self_buffer = bswap64(self_buffer);
		if (self_buffer_bits < 8) {
			flags |= EOF_FLAG;
		}
		self_buffer_bits <<= 3;
	}
	return (flags&&(!self_buffer_bits));
}

inline uint64_t BitStream::ReadExpGolomU()
{
	uint64_t bits = ReadZeros() + 1;
	uint64_t left = bits;
	uint64_t rtn = 0;
	while (left) {
		uint64_t read;
		left -= ReadBits(std::min(left, 64Ui64), read);
		//uint64 cannot hold more data, log an error if needed
		if (left != 0);
		rtn = read;
	}
	return rtn - 1;
}

inline int64_t BitStream::ReadExpGolomS()
{
	uint64_t cur = ReadExpGolomU();
	if (cur & 0x01) {
		return (cur + 1) >> 1;
	}
	return -(int64_t)(cur >> 1);
}




