#pragma comment(lib,"d3d11.lib")
#include <Windows.h>
#include <d3d11.h>
#include "TASTools.h"
#include "TF2Binds.h"
#include <chrono>
#include <iostream>
#include "include/MinHook.h"
#include "InputHooker.h"

using namespace std;

typedef HRESULT(__stdcall* D3D11PRESENT) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
static D3D11PRESENT hookedD3D11Present = nullptr;

auto flipTimestamp = std::chrono::steady_clock::now();
bool flip;

void TASProcessInputProc(UINT &uMsg, WPARAM &wParam, LPARAM &lParam) {
	bool onGround = *(bool*)((uintptr_t)GetModuleHandle("client.dll") + 0x11EED78);

	if (uMsg == WM_KEYDOWN) {
		//int scanCode = (lParam >> 16) & 0xFF;
		//cout << scanCode << endl;
		if (wParam == forwardBinds[0] || wParam == forwardBinds[1]) {
			simulateKeyDown(leftBinds[0]);
			//wParam = leftBinds[0];
			if (!onGround) {
				//uMsg = WM_NULL;
			}
		}
	}
}

void TASProcessXInput(XINPUT_STATE* pState) {
	float velX = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C2C);
	float velY = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C30);

	float yaw = *(float*)((uintptr_t)GetModuleHandle("engine.dll") + 0x7B6668);

	float toDegrees = (180.0f / 3.14159265f);
	float toRadians = (3.14159265f / 180.0f);
	float magnitude = sqrt(pow(velX, 2) + pow(velY, 2));

	auto elapsed = chrono::steady_clock::now() - flipTimestamp;
	long long since = chrono::duration_cast<chrono::milliseconds>(elapsed).count();
	if (since > 100) {
		flip = !flip;
	}

	if (magnitude > 100) {
		float airSpeed = 50;
		float velDegrees = atan2f(velY, velX) * toDegrees;
		float velDirection = yaw - velDegrees;

		float rightx = sinf((velDirection + 90) * toRadians);
		float righty = cosf((velDirection + 90) * toRadians);

		float offsetx = velX + rightx * airSpeed;
		float offsety = velY + righty * airSpeed;

		float angle = atan2f(offsety, offsetx) - atan2f(velY, velX);
		float offsetAngle = atan2f(righty, rightx) - angle;
		cout << offsetAngle << endl;

		short tx = cosf(offsetAngle * toRadians) * 32767.0f;
		short ty = sinf(offsetAngle * toRadians) * 32767.0f;
		//cout << tx << " : " << ty << endl;

		pState->Gamepad.sThumbLX = tx;
		pState->Gamepad.sThumbLY = ty;
	}
}

bool pressingLurch = false;

HRESULT __stdcall detourD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
	/*if (!pressingLurch) {
		simulateKeyDown(forwardBinds[0]);
		pressingLurch = true;
	}
	else {
		simulateKeyUp(forwardBinds[0]);
		pressingLurch = false;
	}*/
	return hookedD3D11Present(pSwapChain, SyncInterval, Flags);
}

BOOL CALLBACK enumWindowsCallback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id)
	{
		return TRUE;
	}
	data.best_handle = handle;
	return FALSE;
}

void hookDirectXPresent() {
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	unsigned long pid = GetCurrentProcessId();
	handle_data data;
	data.process_id = pid;
	data.best_handle = 0;
	EnumWindows(enumWindowsCallback, (LPARAM)&data);

	swapChainDesc.OutputWindow = data.best_handle;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	ID3D11Device* pTmpDevice = NULL;
	ID3D11DeviceContext* pTmpContext = NULL;
	IDXGISwapChain* pTmpSwapChain;
	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, &pTmpSwapChain, &pTmpDevice, NULL, &pTmpContext)))
	{
		cout << "Failed to create directX device and swapchain!" << endl;
		return;
	}

	__int64* pSwapChainVtable = NULL;
	__int64* pDeviceContextVTable = NULL;
	pSwapChainVtable = (__int64*)pTmpSwapChain;
	pSwapChainVtable = (__int64*)pSwapChainVtable[0];
	pDeviceContextVTable = (__int64*)pTmpContext;
	pDeviceContextVTable = (__int64*)pDeviceContextVTable[0];

	if (MH_CreateHook((LPBYTE)pSwapChainVtable[8], &detourD3D11Present, reinterpret_cast<LPVOID*>(&hookedD3D11Present)) != MH_OK)
	{
		cout << "Hooking Present failed!" << endl;
	}
	if (MH_EnableHook((LPBYTE)pSwapChainVtable[8]) != MH_OK)
	{
		cout << "Enabling of Present hook failed!" << endl;
	}
	else {
		cout << "Hooked D3DPresent!" << endl;
	}

	pTmpDevice->Release();
	pTmpContext->Release();
	pTmpSwapChain->Release();
}