#pragma once

#include "Decoder.h"
#include "DX9Globals.h"
#include "Win32Processor.h"

class DXVA2Decoder:public Decoder {
public:
	DXVA2Decoder(size_t& err, decoder_profile& prof);
	virtual ~DXVA2Decoder() override;
	virtual void Reset() override;
private:
	DX9Globals globs;
	IDirectXVideoDecoderService* decode_service = nullptr;
	DXVA2_ConfigPictureDecode dxva_config{};
	GUID dxva_profile = {};
	IDirect3DSurface9** framebuffers = nullptr;
	uint32_t num_framebuffers = 0;
	IDirectXVideoDecoder* decoder = nullptr;
	DXVAHDProcessor* post_proc = nullptr;
};

struct DXVA_profile {
	GUID prof;
	bool supported;
};

//#ifndef WIN32DecoderCPP
extern const DXVA_profile DXVA_codecs[][7];
//extern const GUID DXVA2_ModeH264_VLD_WithFMOASO_NoFGT;
//extern const GUID DXVA2_ModeVC1_D2010;
//#endif
constexpr GUID GUID_NULL{};



