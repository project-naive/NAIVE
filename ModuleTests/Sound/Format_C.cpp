void Float32NE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	int32_t* restrict src = (int32_t*)_src;
	int32_t* restrict dest = (int32_t*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += src[i];
	}
}

void Float32FE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	int32_t* restrict src = (int32_t*)_src;
	int32_t* restrict dest = (int32_t*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += (int32_t)SwapBytes32(src[i]);
	}
}

void Float64NE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	double* restrict src = (double*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += float(src[i]);
	}
}

void Float64FE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	double* restrict src = (double*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		double val = (double)SwapBytes64(src[i]);
		dest[i] += float(val);
	}
}

void U32NE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	uint32_t* restrict src = (uint32_t*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += (src[i] / (0x7FFFFFFF + 0.5f)) - 1.0f;
	}
}

void U32FE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	uint32_t* restrict src = (uint32_t*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += (SwapBytes32(src[i]) / (0x7FFFFFFF + 0.5f)) - 1.0f;
	}
}

void S32NE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	uint32_t* restrict src = (uint32_t*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += ((src[i]) + 0.5f) / (0x7FFFFFFF + 0.5f);
	}
}

void S32FE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	uint32_t* restrict src = (uint32_t*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += (SwapBytes32(src[i]) + 0.5f) / (0x7FFFFFFF + 0.5f);
	}
}

//24 bit will assume input of also 4-byte aligned (not usb, or alredy converted)
void U24NE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	uint32_t* restrict src = (uint32_t*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += (src[i] / (0x007FFFFF + 0.5f)) - 1.0f;
	}
}

//24 bit will assume input of also 4-byte aligned (not usb, or alredy converted)
void U24FE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	uint32_t* restrict src = (uint32_t*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += (SwapBytes32(src[i]) / (0x007FFFFF + 0.5f)) - 1.0f;
	}
}

//24 bit will assume input of also 4-byte aligned (not usb, or alredy converted)
void S24NE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	uint32_t* restrict src = (uint32_t*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += (src[i] + 0.5f) / (0x007FFFFF + 0.5f);
	}
}

//24 bit will assume input of also 4-byte aligned (not usb, or alredy converted)
void S24FE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	uint32_t* restrict src = (uint32_t*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += (SwapBytes32(src[i]) + 0.5f) / (0x007FFFFF + 0.5f);
	}
}

void U16NE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	uint16_t* restrict src = (uint16_t*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += (src[i] / (0x7FFF + 0.5f)) - 1.0f;
	}
}

void U16FE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	uint16_t* restrict src = (uint16_t*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += (SwapBytes16(src[i]) / (0x7FFF + 0.5f)) - 1.0f;
	}
}

void S16NE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	uint16_t* restrict src = (uint16_t*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += (src[i] + 0.5f) / (0x7FFF + 0.5f);
	}
}

void S16FE_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	uint16_t* restrict src = (uint16_t*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += (SwapBytes16(src[i]) + 0.5f) / (0x7FFF + 0.5f);
	}
}

void U8_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	uint8_t* restrict src = (uint8_t*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += (src[i] / (0x7F + 0.5f)) - 1.0f;
	}
}

void S8_to_Float32NE_C(void* restrict _src, void* restrict _dest, int num) {
	int8_t* restrict src = (int8_t*)_src;
	float* restrict dest = (float*)_dest;
	for (int i = 0; i < num; ++i) {
		dest[i] += (src[i] + 0.5f) / (0x7f + 0.5f);
	}
}



