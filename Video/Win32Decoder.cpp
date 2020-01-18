#define WIN32DecoderCPP
#include "Win32Decoder.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(ptr) if(ptr) { ptr->Release(); ptr=nullptr; }
#endif
void DXVA2Decoder::Reset()
{
	SAFE_RELEASE(decoder);
	for (int i = 0; i < num_framebuffers; ++i) {
		SAFE_RELEASE(framebuffers[i]);
	}
	if (framebuffers)
		free(framebuffers);
	framebuffers = nullptr;
	num_framebuffers = 0;
	dxva_profile = GUID_NULL;
	ZeroMemory(&dxva_config, sizeof(dxva_config));
	globs.endDecodeService();
	Decoder::Reset();
}

DXVA2Decoder::~DXVA2Decoder()
{
	Reset();
};

#ifdef FAIL_ON
#undef FAIL_ON
#endif
#define FAIL_ON(hr, rtn) if(FAILED(hr)) { rtn=hr; this->~DXVA2Decoder(); return; }
DXVA2Decoder::DXVA2Decoder(size_t& err, decoder_profile& prof):Decoder(prof), globs()
{
	//init d3d and services
	err = 0;
	FAIL_ON(globs.getDecoderService(&decode_service), err);
	//create decoder
	UINT count;
	GUID* guids;
	FAIL_ON(decode_service->GetDecoderDeviceGuids(&count, &guids), err);
	D3DFORMAT found_format = D3DFMT_UNKNOWN;
	D3DFORMAT target_format = (D3DFORMAT)profile.OUT_FORMAT_FOURCC;
	bool need_conversion = false;
start_color_config:
	for (int i = 0; !IsEqualGUID(DXVA_codecs[uint32_t(profile.codec)][i].prof, GUID_NULL); ++i) {
		if(!DXVA_codecs[uint32_t(profile.codec)][i].supported) continue;
		int j = 0;
		for (; j < count; ++j) {
			if (IsEqualGUID(DXVA_codecs[uint32_t(profile.codec)][i].prof, guids[j])) {
				break;
			}
		}
		if (j == count) continue;
		UINT target_count;
		D3DFORMAT* sample_formats;
		if (FAILED(decode_service->GetDecoderRenderTargets(guids[j], &target_count, &sample_formats)))
			continue;
		found_format = D3DFMT_UNKNOWN;
		for (int k = 0; k < target_count; k++) {
			if (sample_formats[k] == target_format) {
				found_format = sample_formats[k];
				break;
			}
		}
		CoTaskMemFree(sample_formats);
		if (found_format) {
			dxva_profile = guids[j];
			//currently no conversion is needed since
			//there is only one implementation
			profile.codec_profile = j;
			break;
		}
	}
	if (IsEqualGUID(dxva_profile, GUID_NULL) && target_format != profile.IN_FORMAT_FOURCC) {
		target_format = (D3DFORMAT)profile.IN_FORMAT_FOURCC;
		//flag the video as needing a layer of color conversion
		need_conversion = true;
		goto start_color_config;
	}
	CoTaskMemFree(guids);
	if (IsEqualGUID(dxva_profile, GUID_NULL)) {
		FAIL_ON(1, err);
	}
	DXVA2_VideoDesc desc{};
	desc.Format = target_format;
	desc.InputSampleFreq.Numerator = profile.in_sample_rate.numerator;
	desc.InputSampleFreq.Denominator = profile.in_sample_rate.denominator;
	desc.OutputFrameFreq.Numerator = profile.out_frame_rate.numerator;
	desc.OutputFrameFreq.Denominator = profile.out_frame_rate.denominator;
	desc.SampleWidth = profile.samplewidth;
	desc.SampleHeight = profile.sampleheight;
	DXVA2_ConfigPictureDecode* config_decode = nullptr;
	FAIL_ON(decode_service->GetDecoderConfigurations(dxva_profile, &desc, nullptr, &count, &config_decode), err);
	int selected = 0;
	int score = 0;
	for (int i = 0; i < count; ++i) {
		int cur_score;
		if (config_decode[i].ConfigBitstreamRaw == 1)
			cur_score = 1;
		else if (profile.codec == vcodec::vcodec_h264 && config_decode[i].ConfigBitstreamRaw == 2)
			cur_score = 2;
		else {
			//log unkown info
			continue;
		}
		if (IsEqualGUID(config_decode[i].guidConfigBitstreamEncryption, DXVA2_NoEncrypt))
			cur_score += 16;
		if (cur_score > score) {
			score = cur_score;
			selected = i;
		}
	}
	if (!score) {
		//config not found, abort
		CoTaskMemFree(config_decode);
		FAIL_ON(2, err);
	}
	dxva_config = config_decode[selected];
	CoTaskMemFree(config_decode);
	num_framebuffers = 4;
	if (profile.codec == vcodec::vcodec_h264 || profile.codec == vcodec::vcodec_hevc)
		num_framebuffers += 16;
	else if (profile.codec == vcodec::vcodec_vp9)
		num_framebuffers += 8;
	else
		num_framebuffers += 2;
	framebuffers = (IDirect3DSurface9**)malloc(num_framebuffers * (sizeof(IDirect3DSurface9*)));
	if (!framebuffers) FAIL_ON(3, err);
	FAIL_ON(decode_service->CreateSurface(desc.SampleWidth, desc.SampleHeight, num_framebuffers - 1,
		(D3DFORMAT)profile.OUT_FORMAT_FOURCC, D3DPOOL_DEFAULT, 0, DXVA2_VideoDecoderRenderTarget, framebuffers, nullptr), err);
	FAIL_ON(decode_service->CreateVideoDecoder(dxva_profile, &desc, &dxva_config, framebuffers, num_framebuffers, &decoder), err);
	if (need_conversion) {
		process_profile pro;
		pro.frame_corre = DXVAHD_FRAME_FORMAT_PROGRESSIVE;
		pro.in_frame_rate.numerator = profile.out_frame_rate.numerator;
		pro.in_frame_rate.denominator = profile.out_frame_rate.denominator;
		pro.out_frame_rate.numerator = profile.out_frame_rate.numerator;
		pro.out_frame_rate.denominator = profile.out_frame_rate.denominator;
		pro.in_height = profile.frameheight;
		pro.in_width = profile.framewidth;
		pro.out_height = profile.frameheight;
		pro.out_width = profile.framewidth;
		pro.speed_quality = DXVAHD_DEVICE_USAGE_PLAYBACK_NORMAL;
		pro.in_pixfmt = target_format;
		pro.out_pixfmt = (D3DFORMAT)prof.OUT_FORMAT_FOURCC;
		post_proc = new DXVAHDProcessor(err, globs, pro);
	}
	prof = profile;
}

const DXVA_profile DXVA_codecs[][7] = {
	{},
	{{DXVA2_ModeMPEG2and1_VLD, }, {DXVA2_ModeMPEG1_VLD, }},
	{{DXVA2_ModeMPEG2and1_VLD, }, {DXVA2_ModeMPEG2_VLD, }, {DXVA2_ModeMPEG2_IDCT, }, {DXVA2_ModeMPEG2_MoComp,}},
	{{DXVA2_ModeWMV8_MoComp, }, {DXVA2_ModeWMV8_PostProc, }},
	{{DXVA2_ModeWMV9_IDCT, }, {DXVA2_ModeWMV9_MoComp, }, {DXVA2_ModeWMV9_PostProc, }},
	{{DXVA2_ModeVC1_D2010, }, {DXVA2_ModeVC1_VLD, }, {DXVA2_ModeVC1_IDCT, }, {DXVA2_ModeVC1_MoComp, }, {DXVA2_ModeVC1_PostProc, }},
	{{DXVA2_ModeMPEG4pt2_VLD_AdvSimple_GMC, }, {DXVA2_ModeMPEG4pt2_VLD_AdvSimple_NoGMC, }, {DXVA2_ModeMPEG4pt2_VLD_Simple, }},
	{
		{DXVA2_ModeH264_VLD_WithFMOASO_NoFGT, true}, {DXVA2_ModeH264_VLD_FGT, true}, {DXVA2_ModeH264_VLD_NoFGT, true}, 
		{DXVA2_ModeH264_IDCT_FGT, }, {DXVA2_ModeH264_IDCT_NoFGT, }, {DXVA2_ModeH264_MoComp_FGT,}, {DXVA2_ModeH264_MoComp_NoFGT,}
	},
	{{DXVA2_ModeVP8_VLD, }},
	{{DXVA2_ModeHEVC_VLD_Main,}, {DXVA2_ModeHEVC_VLD_Main10,}},
	{{DXVA2_ModeVP9_VLD_Profile0,}, {DXVA2_ModeVP9_VLD_10bit_Profile2,}},
};


