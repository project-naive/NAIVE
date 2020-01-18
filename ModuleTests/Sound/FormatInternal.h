#pragma once

#if defined(_MSC_VER)
#include <intrin.h>
#define restrict __restrict
#define SwapBytes64(x) _byteswap_uint64((unsigned __int64)x)
#define SwapBytes32(x) _byteswap_ulong((unsigned __int32)x)
#define SwapBytes16(x) _byteswap_ushort((unsigned short)x)
#elif defined(__GNUC__) || defined(__GNUG__)
#define restrict __restrict__
inline uint16_t _SwapBytes16(uint32_t value) {
	uint16_t Hi = value << 8;
	uint16_t Lo = value >> 8;
	return Hi | Lo;
}
#define SwapBytes64(x) __builtin_bswap64((uint64_t)x)
#define SwapBytes32(x) __builtin_bswap32((uint32_t)x)
#define SwapBytes16(x) _SwapBytes16((uint16_t)x)
#elif defined(__clang__)
#define restrict
inline uint16_t _SwapBytes16(uint32_t value) {
	uint16_t Hi = value << 8;
	uint16_t Lo = value >> 8;
	return Hi | Lo;
}
#define SwapBytes64(x) __builtin_bswap64((uint64_t)x)
#define SwapBytes32(x) __builtin_bswap32((uint32_t)x)
#define SwapBytes16(x) _SwapBytes16((uint16_t)x)
#else
#define restrict
inline uint64_t _SwapBytes64(uint64_t value) {
	uint32_t Byte0 = value & 0x000000FF;
	uint32_t Byte1 = value & 0x0000FF00;
	uint32_t Byte2 = value & 0x00FF0000;
	uint32_t Byte3 = value & 0xFF000000;
	uint64_t high = (Byte0 << 24) | (Byte1 << 8) | (Byte2 >> 8) | (Byte3 >> 24) << 32;
	value >>= 32;
	uint32_t Byte0 = value & 0x000000FF;
	uint32_t Byte1 = value & 0x0000FF00;
	uint32_t Byte2 = value & 0x00FF0000;
	uint32_t Byte3 = value & 0xFF000000;
	uint64_t low = (Byte0 << 24) | (Byte1 << 8) | (Byte2 >> 8) | (Byte3 >> 24);
	return high | low;
}
inline uint32_t _SwapBytes32(uint32_t src) {
	uint32_t Byte0 = value & 0x000000FF;
	uint32_t Byte1 = value & 0x0000FF00;
	uint32_t Byte2 = value & 0x00FF0000;
	uint32_t Byte3 = value & 0xFF000000;
	return (Byte0 << 24) | (Byte1 << 8) | (Byte2 >> 8) | (Byte3 >> 24);
}
inline uint16_t _SwapBytes16(uint16_t value) {
	uint16_t Hi = value << 8;
	uint16_t Lo = value >> 8;
	return Hi | Lo;
}
#define SwapBytes64(x) _SwapBytes64((uint64_t)x)
#define SwapBytes32(x) _SwapBytes32((uint32_t)x)
#define SwapBytes16(x) _SwapBytes16((uint16_t)x)
#endif

