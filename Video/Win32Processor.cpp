#include "Win32Processor.h"
#include <algorithm>

#define RETURN_CODE_IF(test,code,rtn) if(test) {rtn=code; Reset(); return;}
#define RETURN_CODE_AND_FREE_IF(test,code,ptr,rtn) if(test) {rtn=code; free(ptr); Reset(); return;}
#ifdef FAIL_ON
#undef FAIL_ON
#endif
#define FAIL_ON(hr, rtn) rtn = hr; if(FAILED(rtn)) {Reset(); return;}
#define FAIL_FREE_RETURN(hr, ptr, rtn) rtn = hr; if(FAILED(rtn)) {free(ptr); Reset(); return;}

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(ptr) if(ptr) { ptr->Release(); ptr=nullptr; }
#endif

DXVAHDProcessor::DXVAHDProcessor(size_t& err, const DX9Globals& globals, process_profile& profile): globs(globals)
{
	IDirect3DDevice9Ex* d3d9_device = globals.getD3D9device();
	DXVAHD_CONTENT_DESC desc{};
	desc.InputFrameFormat = profile.frame_corre;
	desc.InputFrameRate.Denominator = profile.in_frame_rate.denominator;
	desc.InputFrameRate.Numerator = profile.in_frame_rate.numerator;
	desc.OutputFrameRate.Denominator = profile.out_frame_rate.denominator;
	desc.OutputFrameRate.Numerator = profile.out_frame_rate.numerator;
	desc.InputHeight = profile.in_height;
	desc.InputWidth = profile.in_width;
	desc.OutputHeight = profile.out_height;
	desc.OutputWidth = profile.out_width;
	FAIL_ON(DXVAHD_CreateDevice(d3d9_device, &desc, profile.speed_quality, nullptr, &dxvahd_device), err);
	FAIL_ON(dxvahd_device->GetVideoProcessorDeviceCaps(&device_caps), err);
	D3DFORMAT* formats = (D3DFORMAT*)malloc(std::max(device_caps.InputFormatCount, device_caps.OutputFormatCount) * sizeof(D3DFORMAT));
	if (!formats) {
		FAIL_ON(1, err);
	}
	HRESULT hr;
	FAIL_FREE_RETURN(dxvahd_device->GetVideoProcessorInputFormats(device_caps.InputFormatCount, formats), formats, err);
	int i;
	for (i = 0; i < device_caps.InputFormatCount; ++i) {
		if (formats[i] == profile.in_pixfmt) break;
	}
	if (i == device_caps.InputFormatCount) {
		free(formats);
		FAIL_ON(2, err);
	}
	FAIL_FREE_RETURN(dxvahd_device->GetVideoProcessorOutputFormats(device_caps.OutputFormatCount, formats), formats, err);
	for (i = 0; i < device_caps.OutputFormatCount; ++i) {
		if (formats[i] == profile.out_pixfmt) break;
	}
	if (i == device_caps.OutputFormatCount) {
		free(formats);
		FAIL_ON(3, err);
	}
	free(formats);
	DXVAHD_VPCAPS* vp_caps = (DXVAHD_VPCAPS*)malloc(device_caps.VideoProcessorCount*sizeof(DXVAHD_VPCAPS));
	if (!vp_caps) {
		FAIL_ON(4, err);
	}
	FAIL_FREE_RETURN(dxvahd_device->GetVideoProcessorCaps(device_caps.VideoProcessorCount, vp_caps), vp_caps, err);
	for (int i = 0; i < device_caps.VideoProcessorCount; ++i) {
		//examine the capabilities and select
	}
	//just select the first one for now.
	processor_caps = vp_caps[0];
	free(vp_caps);
	FAIL_ON(dxvahd_device->CreateVideoProcessor(&processor_caps.VPGuid, &processor), err);
}

void DXVAHDProcessor::Reset()
{
	SAFE_RELEASE(processor);
	SAFE_RELEASE(dxvahd_device);
}
