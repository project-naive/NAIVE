#pragma once

#include "DX9Globals.h"
#include <dxvahd.h>
#undef min
#undef max

struct process_profile {
	struct positive_rational {
		uint32_t numerator;
		uint32_t denominator;
	};
	positive_rational in_frame_rate;
	positive_rational out_frame_rate;
	uint32_t in_width;
	uint32_t in_height;
	uint32_t out_width;
	uint32_t out_height;
	DXVAHD_FRAME_FORMAT frame_corre;
	DXVAHD_DEVICE_USAGE speed_quality;
	D3DFORMAT in_pixfmt;
	D3DFORMAT out_pixfmt;
};


class DXVAHDProcessor {
public:
	DX9Globals globs;
	IDXVAHD_Device* dxvahd_device = nullptr;
	DXVAHD_VPDEVCAPS device_caps;
	DXVAHD_VPCAPS processor_caps;
	IDXVAHD_VideoProcessor* processor;
	DXVAHD_STREAM_DATA data;
	DXVAHDProcessor(size_t& err, const DX9Globals& globals, process_profile& profile);
	void Reset();
};



