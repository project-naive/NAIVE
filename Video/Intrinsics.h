#pragma once

#ifndef INTRINSICS_H
#define INTRINSICS_H
#include <cstdint>
#ifdef _MSC_VER
#include <cstdlib>
#define bswap32(x) _byteswap_ulong(x)
#define bswap64(x) _byteswap_uint64(x)
#include <intrin.h>
#define bscan32f(x, res) _BitScanForward((unsigned long*)&res, (unsigned long)x)
#define bscan64f(x, res) _BitScanForward64((unsigned long*)&res, (unsigned long long)x)
#define bscan32r(x, res) _BitScanReverse((unsigned long*)&res, (unsigned long)x)
#define bscan64r(x, res) _BitScanReverse64((unsigned long*)&res, (unsigned long long)x)
#elif defined(__GNUC__) || defined(__GNUG__)
#define bswap32(x) __builtin_bswap32(x)
#define bswap64(x) __builtin_bswap64(x)
//assuming 4-bytes and 8-byte types
#define bscan32f(x, res) res = __builtin_ctz((unsigned)x)
#define bscan64f(x, res) res = __builtin_ctzll((unsigned long long)x)
#define bscan32r(x, res) res = __builtin_clz((unsigned)x)
#define bscan64r(x, res) res = __builtin_clzll((unsigned long long)x)
#else 
inline uint32_t bswap32(uint32_t in)
{
	return(uint32_t(0xff000000Ui32 & in) >> 24)
		| (uint32_t(0x00ff0000Ui32 & in) >> 8)
		| (uint32_t(0x0000ff00Ui32 & in) << 8)
		| (uint32_t(0x000000ffUi32 & in) << 24);
}
inline uint64_t bswap64(uint64_t in)
{
	return(uint64_t(0xff00000000000000Ui64 & in) >> 56)
		| (uint64_t(0x00ff000000000000Ui64 & in) >> 40)
		| (uint64_t(0x0000ff0000000000Ui64 & in) >> 24)
		| (uint64_t(0x000000ff00000000Ui64 & in) >> 8)
		| (uint64_t(0x00000000ff000000Ui64 & in) << 8)
		| (uint64_t(0x0000000000ff0000Ui64 & in) << 24)
		| (uint64_t(0x000000000000ff00Ui64 & in) << 40)
		| (uint64_t(0x00000000000000ffUi64 & in) << 56);
}
inline void bscan32f(uint32_t x, uint32_t& res)
{
	uint32_t rtn = 0;
	for (; rtn < 32; ++rtn) {
		if (x & (0x80000000Ui32 >> rtn)) break;
	}
	res = rtn;
}
inline void bscan64f(uint64_t x, uint32_t& res)
{
	uint32_t rtn = 0;
	for (; rtn < 64; ++rtn) {
		if (x & (0x8000000000000000Ui64 >> rtn)) break;
	}
	res = rtn;
}
inline void bscan32r(uint32_t x, uint32_t& res)
{
	uint32_t rtn = 0;
	for (; rtn < 32; ++rtn) {
		if (x & (0x00000001Ui32 << rtn)) break;
	}
	res = rtn;
}
inline void bscan64r(uint64_t x, uint32_t& res)
{
	uint32_t rtn = 0;
	for (; rtn < 64; ++rtn) {
		if (x & (0x0000000000000001Ui64 << rtn)) break;
	}
	res = rtn;
}
#endif

#endif