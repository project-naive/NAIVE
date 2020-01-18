#pragma once
#include "Win32Decoder.h"

#include <dxva.h>
#include <dxva2api.h>


#ifdef FAIL_IF
#undef FAIL_IF
#endif
#ifdef FAIL_AND_FREE_IF
#undef FAIL_AND_FREE_IF
#endif
#ifdef FAIL_ON
#undef FAIL_ON
#endif
#ifdef FAIL_FREE_ON
#undef FAIL_FREE_ON
#endif
#define FAIL_IF(test, rtn) rtn = test; if(rtn) {Reset(); return;}
#define FAIL_AND_FREE_IF(test,ptr,rtn)  rtn = test; if(test) {(ptr); Reset(); return;}
#define FAIL_ON(hr, rtn) rtn = hr; if(FAILED(rtn)) {Reset(); return;}
#define FAIL_FREE_ON(hr, ptr, rtn) rtn = hr; if(FAILED(rtn)) {free(ptr); Reset(); return;}

struct DXVAH264Buffers {
	enum {
		OFFHost_BParse,
		OFFHost_Compose,
		Deblock_filter,
		FilmGrain,
		Status
	};
	uint32_t type;
	virtual HRESULT send_buffers(void* data) = 0;
};

struct DXVAH264BuffersBParse: DXVAH264Buffers {
	DXVA_Qmatrix_H264 qmatrix_data;
	DXVA_PicParams_H264 pic_params;
	//short is just the begining part
	DXVA_Slice_H264_Long slice_control;
	//bitstream is given directly to the ...
};

class DXVA2H264Decoder final: public DXVA2Decoder  {
	enum codec_profile:uint32_t {
		profile_VLD_FMOASO_NoFGT,
		profile_VLD_FGT,
		profile_VLD_NoFGT,
		profile_unknown,
	};

	DXVA2H264Decoder(size_t& err,decoder_profile& info):DXVA2Decoder(err, info)
	{
		FAIL_IF(verify_profile(info),err);
	}
	virtual ~DXVA2H264Decoder() {
		Reset();
	}
	void Reset()
	{
		DXVA2Decoder::Reset();
	}
	static HRESULT verify_profile(decoder_profile& prof)
	{
		if (prof.codec != vcodec::vcodec_h264) {
			return 10;
		}
		if (prof.codec_profile >= codec_profile::profile_unknown) {
			return 11;
		}
		//other things
	}
};

