#define D3D_DEBUG_INFO
#include <initguid.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <dxvahd.h>
#undef min
#undef max
#include <iostream>
#include <algorithm>

bool open = true;

#define IF_FAILED_GOTO(hr, label) if (FAILED(hr)) { goto label; }
#define CHECK_HR(hr) IF_FAILED_GOTO(hr, done)
template <class T>
inline void SAFE_RELEASE(T*& p)
{
    if (p)
    {
        p->Release();
        p = NULL;
    }
}

LRESULT CALLBACK MainWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam)    // second message parameter
{

    switch (uMsg)
    {
    case WM_CREATE:
        DefWindowProc(hwnd, uMsg, wParam, lParam);
        // Initialize the window. 
        return 0;

    case WM_PAINT:
        DefWindowProc(hwnd, uMsg, wParam, lParam);
        // Paint the window's client area. 
        return 0;

    case WM_SIZE:
        DefWindowProc(hwnd, uMsg, wParam, lParam);
        // Set the size and position of the window. 
        return 0;

    case WM_DESTROY:
        DefWindowProc(hwnd, uMsg, wParam, lParam);
        // Clean up window-specific data objects. 
        return 0;

        // 
        // Process other messages. 
        // 

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

/*
int main() {
    CoInitialize(NULL);
	const wchar_t CLASS_NAME[] = L"Dummy Window Class";
	WNDCLASS wc = {};
	wc.lpszClassName = CLASS_NAME;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    RegisterClass(&wc);
    HWND m_hwnd = CreateWindowEx(0, CLASS_NAME, L"Main Window", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, wc.hInstance, nullptr);
    ShowWindow(m_hwnd, SW_SHOW);
    IDirect3D9Ex* m_pD3D9 = nullptr;
    HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION,&m_pD3D9);
    D3DPRESENT_PARAMETERS present{};
    present.BackBufferWidth = 1;
    present.BackBufferHeight = 1;
    present.BackBufferFormat = D3DFMT_X8R8G8B8;
    present.Windowed = true;
    present.hDeviceWindow = m_hwnd;
    present.Flags = D3DPRESENTFLAG_VIDEO;
    present.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    present.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    present.SwapEffect = D3DSWAPEFFECT_DISCARD;
    IDirect3DDevice9Ex* m_D3D9device = nullptr;
    hr=m_pD3D9->CreateDeviceEx(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        m_hwnd,
        D3DCREATE_NOWINDOWCHANGES | D3DCREATE_MULTITHREADED | D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &present,
        nullptr,
        &m_D3D9device
    );
    UINT reset_token;
    IDirect3DDeviceManager9* manager=nullptr;
    hr=DXVA2CreateDirect3DDeviceManager9(&reset_token,&manager);
    hr=manager->ResetDevice(m_D3D9device,reset_token);
    HANDLE device9=nullptr;
    manager->OpenDeviceHandle(&device9);
    IDirectXVideoDecoderService* decode_service=nullptr;
    manager->GetVideoService(device9, IID_IDirectXVideoDecoderService,(void**)&decode_service);
    GUID codec=DXVA2_ModeH264_E;
    DXVA2_VideoDesc dxva2_desc{};
    dxva2_desc.Format=(D3DFORMAT)MAKEFOURCC('N','V','1','2');
    dxva2_desc.InputSampleFreq.Denominator=30000;
    dxva2_desc.InputSampleFreq.Numerator=1001;
    dxva2_desc.OutputFrameFreq.Denominator = 60;
    dxva2_desc.OutputFrameFreq.Numerator = 1;
    dxva2_desc.SampleHeight=1080;
    dxva2_desc.SampleWidth=1920;
    UINT count;
    DXVA2_ConfigPictureDecode* config_decode=nullptr;
    GUID* guids;
    hr= decode_service->GetDecoderDeviceGuids(&count,&guids);
    std::cout<<hr;
    decode_service->GetDecoderConfigurations(codec,&dxva2_desc,nullptr,&count,&config_decode);
    int uiIndex=0;
    int selected=-1;
    DXVA2_ConfigPictureDecode* found=nullptr;
    for (; uiIndex < count; uiIndex++) {

        if (config_decode[uiIndex].ConfigBitstreamRaw == 2) {

            // if ConfigBitstreamRaw == 2, we can use DXVA_Slice_H264_Short
            selected = uiIndex;
            found=config_decode+uiIndex;
            break;
        }
    }
    IDirect3DSurface9* surfs[3];
    for (int i = 0; i < 3; ++i) {
        decode_service->CreateSurface(dxva2_desc.SampleWidth, dxva2_desc.SampleHeight, 0,
            (D3DFORMAT)MAKEFOURCC('N', 'V', '1', '2'), D3DPOOL_DEFAULT, 0, DXVA2_VideoDecoderRenderTarget, &surfs[i], nullptr);
    }
    IDirectXVideoDecoder* decoder=nullptr;
    decode_service->CreateVideoDecoder(codec,&dxva2_desc,&config_decode[uiIndex],surfs,3,&decoder);
    
    system("pause");
    decoder->Release();
}
*/

#include <GLFW/glfw3.h>
#include "Win32Decoder.h"

int main()
{
    glfwInit();
    GLFWwindow* window=glfwCreateWindow(800,600,"Hello",nullptr,nullptr);
    size_t err=0;
    decoder_profile profile;
    profile.codec=vcodec::vcodec_h264;
    profile.frameheight=1080;
    profile.framewidth=1920;
    profile.frame_refs=10;
    profile.IN_FORMAT_FOURCC=MAKEFOURCC('N','V','1','2');
    profile.OUT_FORMAT_FOURCC=D3DFMT_X8R8G8B8;
    profile.in_sample_rate.denominator=30000;
    profile.in_sample_rate.numerator=1001;
    profile.out_frame_rate.denominator=60;
    profile.out_frame_rate.numerator=1;
    profile.sampleheight=1080;
    profile.samplewidth=1920;
    DX9Globals globals(err,window);
    DXVA2Decoder decoder(err,profile);
    std::cout<<err;
    system("pause");
    glfwDestroyWindow(window);
    glfwTerminate();
}








