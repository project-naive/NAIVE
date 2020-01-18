#pragma once

#include <initguid.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <GLFW/glfw3.h>

class DX9Globals {
	static HWND hwindow;
	static IDirect3D9Ex* d3d9;
	static IDirect3DDevice9Ex* d3d9_device;
	static UINT d3d9manager_reset_token;
	static IDirect3DDeviceManager9* d3d9_manager;
	static HANDLE hdevice;
	static IDirectXVideoDecoderService* decode_service;
	static size_t decode_refcount;
	static size_t ref_count;
	static size_t Init(GLFWwindow* window) noexcept;
public:
	static void ref() noexcept;
	static void unref() noexcept;
	static void Reset() noexcept;
	DX9Globals(size_t& err, GLFWwindow* window = nullptr) noexcept;
	DX9Globals(const DX9Globals& src) noexcept;
	DX9Globals& operator=(const DX9Globals& src)noexcept;
	DX9Globals(DX9Globals&& src)noexcept {}
	DX9Globals& operator=(DX9Globals&&)noexcept {}
	DX9Globals();
	~DX9Globals();
	static HRESULT getDecoderService(IDirectXVideoDecoderService** ptr);
	static void endDecodeService();
	static IDirect3DDevice9Ex* getD3D9device();
};
