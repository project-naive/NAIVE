#include "DX9Globals.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <cassert>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(ptr) if(ptr) { ptr->Release(); ptr=nullptr; }
#endif

#ifdef FAIL_ON
#undef FAIL_ON
#endif
#define FAIL_ON(hr) if(FAILED(hr)) { Reset(); return hr; }

HWND DX9Globals::hwindow = nullptr;
IDirect3D9Ex* DX9Globals::d3d9 = nullptr;
IDirect3DDevice9Ex* DX9Globals::d3d9_device = nullptr;
UINT DX9Globals::d3d9manager_reset_token = 0;
IDirect3DDeviceManager9* DX9Globals::d3d9_manager = nullptr;
HANDLE DX9Globals::hdevice = nullptr;
IDirectXVideoDecoderService* DX9Globals::decode_service = nullptr;
size_t DX9Globals::decode_refcount=0;
size_t DX9Globals::ref_count = 0;

size_t DX9Globals::Init(GLFWwindow* window) noexcept
{
	if (!window)
		FAIL_ON(1);
	hwindow = glfwGetWin32Window(window);
	if (!hwindow)
		FAIL_ON(2);
	FAIL_ON(Direct3DCreate9Ex(D3D_SDK_VERSION, &d3d9));
	D3DPRESENT_PARAMETERS present{};
	present.BackBufferWidth = 1;
	present.BackBufferHeight = 1;
	present.hDeviceWindow = hwindow;
	present.SwapEffect = D3DSWAPEFFECT_FLIP;
	present.Windowed = true;
	present.Flags = D3DPRESENTFLAG_VIDEO;
	FAIL_ON(d3d9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwindow,
		D3DCREATE_NOWINDOWCHANGES | D3DCREATE_MULTITHREADED | D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&present, nullptr, &d3d9_device));
	FAIL_ON(DXVA2CreateDirect3DDeviceManager9(&d3d9manager_reset_token, &d3d9_manager));
	FAIL_ON(d3d9_manager->ResetDevice(d3d9_device, d3d9manager_reset_token));
	FAIL_ON(d3d9_manager->OpenDeviceHandle(&hdevice));
}

void DX9Globals::ref() noexcept
{
	++ref_count;
}

void DX9Globals::unref() noexcept
{
	--ref_count;
}

void DX9Globals::Reset() noexcept
{
	SAFE_RELEASE(decode_service);
	if (d3d9_manager && hdevice) {
		d3d9_manager->CloseDeviceHandle(hdevice);
		hdevice = nullptr;
	}
	SAFE_RELEASE(d3d9_manager);
	SAFE_RELEASE(d3d9_device);
	SAFE_RELEASE(d3d9);
	return;
}

DX9Globals::DX9Globals(size_t& err, GLFWwindow* window) noexcept
{
	err = 0;
	if (!DX9Globals::ref_count) {
		err = DX9Globals::Init(window);
	}
	if (!err) ++ref_count;
}

DX9Globals::DX9Globals(const DX9Globals& src) noexcept
{
	++ref_count;
}

DX9Globals& DX9Globals::operator=(const DX9Globals& src)noexcept
{
	++ref_count;
	return *this;
}

DX9Globals::DX9Globals()
{
	assert(ref_count);
	++ref_count;
}

DX9Globals::~DX9Globals()
{
	--ref_count;
	if (!ref_count) Reset();
}

HRESULT DX9Globals::getDecoderService(IDirectXVideoDecoderService** ptr)
{
	HRESULT hr = 0;
	if (!decode_refcount) {
		hr = d3d9_manager->GetVideoService(hdevice, IID_IDirectXVideoDecoderService, (void**)&decode_service);
		if (FAILED(hr)) return hr;
	}
	++ref_count;
	++decode_refcount;
	*ptr = decode_service;
	return 0;
}

void DX9Globals::endDecodeService()
{
	assert(decode_refcount);
	--decode_refcount;
	--ref_count;
	if (!decode_refcount) {
		decode_service->Release();
		decode_service = nullptr;
	}
}

IDirect3DDevice9Ex* DX9Globals::getD3D9device()
{
	return d3d9_device;
}
