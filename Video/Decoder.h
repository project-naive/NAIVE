#pragma once

#include <cstdint>
#include <cstring>

#pragma warning disable 6001
#pragma warning disable 6387
#pragma warning disable 26812

enum class vcodec:uint32_t {
	vcodec_unknown,
	vcodec_mpeg1,
	vcodec_mpeg2,
	vcodec_wmv8,
	vcodec_wmv9,
	vcodec_vc1,
	vcodec_mpegpt2,
	vcodec_h264,
	vcodec_vp8,
	vcodec_hevc,
	vcodec_vp9,
	vcodec_last
};

struct decoder_profile {
	struct positive_rational {
		uint32_t numerator;
		uint32_t denominator;
	};
	positive_rational in_sample_rate;
	positive_rational out_frame_rate;
	uint32_t IN_FORMAT_FOURCC;
	uint32_t OUT_FORMAT_FOURCC;
	uint32_t samplewidth;
	uint32_t sampleheight;
	uint32_t framewidth;
	uint32_t frameheight;
	uint32_t frame_refs;
	vcodec codec;
	uint32_t codec_profile;
};

class Decoder {
public:
	Decoder(decoder_profile& prof)
	{
		memcpy(&profile, &prof, sizeof(decoder_profile));
	};
	virtual ~Decoder(){
		Reset();
	};
	virtual void Reset(){};
	decoder_profile profile;
};

#include <GLFW/glfw3.h>


#include <initguid.h>
#include <d3d9.h>
#include <dxva2api.h>

struct FOURCCs {
	uint32_t fourccs[7];
};

constexpr FOURCCs codecfourccs[] = {
	{},
	{
		MAKEFOURCC('M', 'P', 'E', 'G'), MAKEFOURCC('M', 'P', 'G', '1')
	},
	{
		MAKEFOURCC('M', 'P', 'G', 'V'), MAKEFOURCC('M', 'P', 'G', '2'), MAKEFOURCC('M', 'P', '2', 'A'),
		MAKEFOURCC('M', 'P', '2', 'T'), MAKEFOURCC('M', 'P', '2', 'V'), MAKEFOURCC('M', '2', 'V', '1')
	},
	{
		MAKEFOURCC('W', 'M', 'V', '2'),
	},
	{
		MAKEFOURCC('W', 'M', 'V', 'A'), MAKEFOURCC('W', 'M', 'V', '3'), MAKEFOURCC('M', 'S', 'S', '2'),
		MAKEFOURCC('W', 'M', 'V', 'P'), MAKEFOURCC('W', 'V', 'P', '2'), MAKEFOURCC('W', 'V', 'C', '1')
	},
	{
		MAKEFOURCC('W', 'V', 'C', '1')
	},
	{

	},
	{
		MAKEFOURCC('X', '2', '6', '4'), MAKEFOURCC('H', '2', '6', '4'), MAKEFOURCC('A', 'V', 'C', '1'),
		MAKEFOURCC('M', 'P', '4', 'V')
	},
	{
		MAKEFOURCC('V', 'P', '8', '0'), MAKEFOURCC('V', 'P', '0', '8')
	},
	{
		MAKEFOURCC('H', 'E', 'V', 'C'), MAKEFOURCC('H', 'V', 'C', '1'), MAKEFOURCC('X', '2', '6', '5'),
		MAKEFOURCC('H', '2', '6', '5')
	},
	{
		MAKEFOURCC('V', 'P', '9', '0'), MAKEFOURCC('V', 'P', '0', '9')
	},
};






