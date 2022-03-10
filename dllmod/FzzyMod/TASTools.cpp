#pragma comment(lib,"d3d11.lib")
#include <Windows.h>
#include <d3d11.h>
#include "TASTools.h"
#include "TF2Binds.h"
#include <chrono>
#include <iostream>
#include "include/MinHook.h"
#include "InputHooker.h"
#include <bitset>

using namespace std;

typedef HRESULT(__stdcall* D3D11PRESENT) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef void(__fastcall* SLIDEFRICTION)();
typedef void(__fastcall* GROUNDFRICTION)();
typedef void(__fastcall* WALLFRICTION)();

static SLIDEFRICTION hookedSlideFriction = nullptr;
static GROUNDFRICTION hookedGroundFriction = nullptr;
static WALLFRICTION hookedWallFriction = nullptr;
static D3D11PRESENT hookedD3D11Present = nullptr;

template <typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
{
	return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

template <typename T>
inline MH_STATUS MH_CreateHookApiEx(LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, T** ppOriginal)
{
	return MH_CreateHookApi(pszModule, pszProcName, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

auto flipTimestamp = std::chrono::steady_clock::now();
bool flip;

bool pressingForward;
bool pressingRight;
bool pressingLeft;
bool onWall;
bool onGround;

bool nldj;

bool TASProcessInputDown(WPARAM& key) {

	if (key == jumpBinds[0] || key == jumpBinds[1]) {
		nldj = true;
		cout << "eat jump" << endl;
		return false;
	}
	if (key == forwardBinds[0] || key == forwardBinds[1]) {
		pressingForward = true;
	}
	if (key == rightBinds[0] || key == rightBinds[1]) {
		pressingRight = true;
		//return false;
	}
	if (key == leftBinds[0] || key == leftBinds[1]) {
		pressingLeft = true;
		//return false;
	}
	return true;
}

bool TASProcessInputUp(WPARAM& key) {
	if (key == forwardBinds[0] || key == forwardBinds[1]) {
		pressingForward = false;
	}
	if (key == rightBinds[0] || key == rightBinds[1]) {
		pressingRight = false;
		//return false;
	}
	if (key == leftBinds[0] || key == leftBinds[1]) {
		pressingLeft = false;
		//return false;
	}
	return true;
}

XINPUT_STATE xState;

void TASProcessXInput(XINPUT_STATE* pState) {
	pState->Gamepad = xState.Gamepad;
}

bool pressingLurch = false;
bool pressingJump;

HRESULT __stdcall detourD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
	bool fullyCrouched = *(float*)((uintptr_t)GetModuleHandle("engine.dll") + 0xF84BDAC) == 38;
	int jump = jumpBinds[0];
	if (jump <= 0x06) jump = jumpBinds[1];
	if (jump <= 0x06) return hookedD3D11Present(pSwapChain, SyncInterval, Flags);
	if (pressingJump) {
		simulateKeyUp(jump);
		if (nldj) {
			if (!pressingRight) simulateKeyUp(rightBinds[0]);
			if (!pressingLeft) simulateKeyUp(leftBinds[0]);
			nldj = false;
		}
	}
	if (nldj) {
		simulateKeyDown(rightBinds[0]);
		simulateKeyDown(leftBinds[0]);
		//simulateKeyDown(jump);
		pressingJump = true;
	}

	if (!onGround && !onWall) {
		if (pressingForward) {
			if (pressingLurch) {
				simulateKeyUp(forwardBinds[0]);
				pressingLurch = false;
			}
			else {
				simulateKeyDown(forwardBinds[0]);
				pressingLurch = true;
			}
		}
	}
	if ((onGround || onWall) && pressingForward && !pressingLurch) {
		simulateKeyDown(forwardBinds[0]);
		pressingLurch = true;
	}

	if (onGround && fullyCrouched) {
		simulateKeyDown(jump);
		pressingJump = true;
	}

	if (onWall) {
		simulateKeyDown(jump);
		pressingJump = true;
	}

	float velX = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C2C);
	float velY = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C30);

	float yaw = *(float*)((uintptr_t)GetModuleHandle("engine.dll") + 0x7B6668);

	float toDegrees = (180.0f / 3.14159265f);
	float toRadians = (3.14159265f / 180.0f);
	float magnitude = sqrt(pow(velX, 2) + pow(velY, 2));

	/*if (!onGround && !onWall) {
		float airSpeed = 50;
		float margin = 10;
		float velDegrees = atan2f(velX, -velY) * toDegrees;
		float velDirection = velDegrees - yaw;

		float rightx = sinf((velDirection + 90) * toRadians);
		float righty = cosf((velDirection + 90) * toRadians);

		float offsetx = velX + rightx * airSpeed;
		float offsety = velY + righty * airSpeed;

		float angle = (atan2f(offsety, offsetx) - atan2f(velY, velX)) * toDegrees;
		//cout << angle << endl;
		//float offsetAngle = atan2f(righty, rightx) - angle;

		float offset = fabsf(asinf(airSpeed - margin) / magnitude) * toDegrees;
		float a = velDirection;
		if (pressingLeft) flip = true;
		if (pressingRight) flip = false;
		if (flip) {
			a += 89.7f;
		}
		else {
			a -= 89.7f;
		}

		short tx = cosf(a * toRadians) * 32767.0f;
		short ty = sinf(a * toRadians) * 32767.0f;

		xState.Gamepad.sThumbLX = tx;
		xState.Gamepad.sThumbLY = ty;
		if (pressingLeft && pressingRight) {
			xState.Gamepad.sThumbLX = 0;
			xState.Gamepad.sThumbLY = 0;
		}
	}
	else {
		xState.Gamepad.sThumbLX = 0;
		xState.Gamepad.sThumbLY = 0;
	}
	flip = !flip;*/

	onWall = false;
	onGround = false;

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

void __fastcall detourSlideFriction() {
	//cout << "slide friction" << endl;
	onGround = true;
	hookedSlideFriction();
}

void __fastcall detourWallFriction() {
	//cout << "slide friction" << endl;
	onWall = true;
	hookedWallFriction();
}

void __fastcall detourGroundFriction() {
	//cout << "slide friction" << endl;
	onGround = true;
	hookedGroundFriction();
}

void setMovementHooks() {
	SLIDEFRICTION slideFriction = SLIDEFRICTION((uintptr_t)GetModuleHandle("client.dll") + 0x1F7573);
	DWORD slideResult = MH_CreateHookEx(slideFriction, &detourSlideFriction, &hookedSlideFriction);
	if (slideResult != MH_OK) {
		cout << "hook slide friction failed" << slideResult << endl;
	}
	GROUNDFRICTION groundFriction = GROUNDFRICTION((uintptr_t)GetModuleHandle("client.dll") + 0x1FA2CF);
	DWORD groundResult = MH_CreateHookEx(groundFriction, &detourGroundFriction, &hookedGroundFriction);
	if (groundResult != MH_OK) {
		cout << "hook ground friction failed" << groundResult << endl;
	}
	WALLFRICTION wallFriction = WALLFRICTION((uintptr_t)GetModuleHandle("client.dll") + 0x20D6E5);
	DWORD wallResult = MH_CreateHookEx(wallFriction, &detourWallFriction, &hookedWallFriction);
	if (wallResult != MH_OK) {
		cout << "hook wall friction failed" << wallResult << endl;
	}
}