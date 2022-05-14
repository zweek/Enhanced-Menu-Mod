//#pragma comment(lib, "winmm.lib")
#include "InputHooker.h"
#include "include/MinHook.h"
#include <iostream>
#include <mutex>
#include <string>
#include <bitset>
#include <cmath>
#include "TF2Binds.h"
#include "SourceConsole.h"
#include "TASTools.h"
#include <d3d11.h>

using namespace std;

typedef DWORD(WINAPI* XINPUTGETSTATE)(DWORD, XINPUT_STATE*);
typedef void(__fastcall* POSTEVENT)(__int64, InputEventType_t, int, ButtonCode_t, ButtonCode_t, int);
typedef HRESULT(__stdcall* D3D11PRESENT) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

static D3D11PRESENT hookedD3D11Present = nullptr;
static POSTEVENT hookedPostEvent = nullptr;
static XINPUTGETSTATE hookedXInputGetState = nullptr;

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

bool consoleEnabled;

/*void playSound() {
	struct stat buffer;
	std::string name = "crouchkick.wav";
	if (stat(name.c_str(), &buffer) == 0) {
		PlaySound("crouchkick.wav", NULL, SND_ASYNC);
	}
}*/

InputHolder jumpPressHolder;
InputHolder jumpReleaseHolder;
InputHolder crouchPressHolder;
InputHolder crouchReleaseHolder;

auto jumptime = std::chrono::steady_clock::now();
auto crouchtime = std::chrono::steady_clock::now();

ControllerInputHolder controllerJumpPressHolder;
ControllerInputHolder controllerCrouchPressHolder;
ControllerInputHolder controllerJumpReleaseHolder;
ControllerInputHolder controllerCrouchReleaseHolder;

bool controllerJumpWasPressed;
bool controllerCrouchWasPressed;

DWORD WINAPI detourXInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	DWORD toReturn = hookedXInputGetState(dwUserIndex, pState);

	//TASProcessXInput(pState);

	//pState->Gamepad.sThumbLX = (short)32767;
	//m_sourceConsole->Print(("LY IMMEDIATE: " + to_string(pState->Gamepad.sThumbLY) + "\n").c_str());

	WORD og = pState->Gamepad.wButtons;
	// Convert wButtons to button code so i can match up binds
	int jumpButtonIndex = 0;
	int crouchButtonIndex = 0;
	for (int i = 0; i < 14; i++) {
		bool bDown = (pState->Gamepad.wButtons & (1 << i)) != 0;
		switch (i) {
		case 0:
			if (controllerJump == KEY_XBUTTON_UP) jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_UP) crouchButtonIndex = i;
			break;
		case 1:
			if (controllerJump == KEY_XBUTTON_DOWN) jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_DOWN) crouchButtonIndex = i;
			break;
		case 2:
			if (controllerJump == KEY_XBUTTON_LEFT) jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_LEFT) crouchButtonIndex = i;
			break;
		case 3:
			if (controllerJump == KEY_XBUTTON_RIGHT) jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_RIGHT) crouchButtonIndex = i;
			break;
		case 4:
			if (controllerJump == KEY_XBUTTON_START) jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_START) crouchButtonIndex = i;
			break;
		case 5:
			if (controllerJump == KEY_XBUTTON_BACK) jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_BACK) crouchButtonIndex = i;
			break;
		case 6:
			if (controllerJump == KEY_XBUTTON_STICK1) jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_STICK1) crouchButtonIndex = i;
			break;
		case 7:
			if (controllerJump == KEY_XBUTTON_STICK2) jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_STICK2) crouchButtonIndex = i;
			break;
		case 8:
			if (controllerJump == KEY_XBUTTON_LEFT_SHOULDER) jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_LEFT_SHOULDER) crouchButtonIndex = i;
			break;
		case 9:
			if (controllerJump == KEY_XBUTTON_RIGHT_SHOULDER) jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_RIGHT_SHOULDER) crouchButtonIndex = i;
			break;
		case 10:
			if (controllerJump == KEY_XBUTTON_A) jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_A) crouchButtonIndex = i;
			break;
		case 11:
			if (controllerJump == KEY_XBUTTON_B) jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_B) crouchButtonIndex = i;
			break;
		case 12:
			if (controllerJump == KEY_XBUTTON_X) jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_X) crouchButtonIndex = i;
			break;
		case 13:
			if (controllerJump == KEY_XBUTTON_Y) jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_Y) crouchButtonIndex = i;
			break;
		}
	}

	// Handle controller jump/crouch buffering
	bool jumpDown = (pState->Gamepad.wButtons & (1 << jumpButtonIndex)) != 0;
	bool crouchDown = (pState->Gamepad.wButtons & (1 << crouchButtonIndex)) != 0;

	// Jump Press
	if (!controllerJumpWasPressed && jumpDown) {
		if (controllerCrouchPressHolder.waitingToSend) {
			controllerCrouchPressHolder.waitingToSend = false;
			//playSound();
			auto crouchElapsed = std::chrono::steady_clock::now() - controllerCrouchPressHolder.timestamp;
			long long sinceCrouch = std::chrono::duration_cast<std::chrono::milliseconds>(crouchElapsed).count();
			m_sourceConsole->Print(("crouchkick: " + to_string(sinceCrouch) + "\n").c_str());
		}
		else {
			controllerJumpPressHolder.waitingToSend = true;
			controllerJumpPressHolder.timestamp = std::chrono::steady_clock::now();
		}
	}
	// Jump Release
	if (controllerJumpWasPressed && !jumpDown) {
		controllerJumpReleaseHolder.waitingToSend = true;
		controllerJumpReleaseHolder.timestamp = std::chrono::steady_clock::now();
	}

	// Crouch Press
	if (!controllerCrouchWasPressed && crouchDown) {
		if (controllerJumpPressHolder.waitingToSend) {
			controllerJumpPressHolder.waitingToSend = false;
			//playSound();
			auto jumpElapsed = std::chrono::steady_clock::now() - controllerJumpPressHolder.timestamp;
			long long sinceJump = std::chrono::duration_cast<std::chrono::milliseconds>(jumpElapsed).count();
			m_sourceConsole->Print(("crouchkick: " + to_string(sinceJump) + "\n").c_str());
		}
		else {
			controllerCrouchPressHolder.waitingToSend = true;
			controllerCrouchPressHolder.timestamp = std::chrono::steady_clock::now();
		}
	}
	// Crouch Release
	if (controllerCrouchWasPressed && !crouchDown) {
		controllerCrouchReleaseHolder.waitingToSend = true;
		controllerCrouchReleaseHolder.timestamp = std::chrono::steady_clock::now();
	}

	if (controllerJumpReleaseHolder.waitingToSend) {
		auto jumpElapsed = std::chrono::steady_clock::now() - controllerJumpReleaseHolder.timestamp;
		long long sinceJump = std::chrono::duration_cast<std::chrono::milliseconds>(jumpElapsed).count();
		if (sinceJump > CROUCHKICK_BUFFERING) {
			controllerJumpReleaseHolder.waitingToSend = false;
		}
		else {
			pState->Gamepad.wButtons = pState->Gamepad.wButtons | (1 << jumpButtonIndex);
		}
	}
	if (controllerCrouchReleaseHolder.waitingToSend) {
		auto crouchElapsed = std::chrono::steady_clock::now() - controllerCrouchReleaseHolder.timestamp;
		long long sinceCrouch = std::chrono::duration_cast<std::chrono::milliseconds>(crouchElapsed).count();
		if (sinceCrouch > CROUCHKICK_BUFFERING) {
			controllerCrouchReleaseHolder.waitingToSend = false;
		}
		else {
			pState->Gamepad.wButtons = pState->Gamepad.wButtons | (1 << crouchButtonIndex);
		}
	}

	if (controllerJumpPressHolder.waitingToSend) {
		auto jumpElapsed = std::chrono::steady_clock::now() - controllerJumpPressHolder.timestamp;
		long long sinceJump = std::chrono::duration_cast<std::chrono::milliseconds>(jumpElapsed).count();
		if (sinceJump > CROUCHKICK_BUFFERING) {
			controllerJumpPressHolder.waitingToSend = false;
		}
		else {
			pState->Gamepad.wButtons = pState->Gamepad.wButtons & ~(1 << jumpButtonIndex);
		}
	}
	if (controllerCrouchPressHolder.waitingToSend) {
		auto crouchElapsed = std::chrono::steady_clock::now() - controllerCrouchPressHolder.timestamp;
		long long sinceCrouch = std::chrono::duration_cast<std::chrono::milliseconds>(crouchElapsed).count();
		if (sinceCrouch > CROUCHKICK_BUFFERING) {
			controllerCrouchPressHolder.waitingToSend = false;
		}
		else {
			pState->Gamepad.wButtons = pState->Gamepad.wButtons & ~(1 << crouchButtonIndex);
		}
	}

	controllerJumpWasPressed = jumpDown;
	controllerCrouchWasPressed = crouchDown;

	//m_sourceConsole->Print(("LX END: " + to_string(pState->Gamepad.sThumbLX) + "\n").c_str());

	return toReturn;
}

void spoofPostEvent(__int64 a, InputEventType_t nType, int nTick, ButtonCode_t scanCode, ButtonCode_t virtualCode, int data3) {
	hookedPostEvent(a, nType, nTick, scanCode, virtualCode, data3);
}

void __fastcall detourPostEvent(__int64 a, InputEventType_t nType, int nTick, ButtonCode_t scanCode, ButtonCode_t virtualCode, int data3) {
	ButtonCode_t key = scanCode;
	//if (TASProcessInput(a, nType, nTick, scanCode, virtualCode, data3)) return;
	if (nType == IE_AnalogValueChanged && scanCode == 3) {
		m_sourceConsole->Print(("scanCode: " + to_string(scanCode) + "   virtualCode: " + to_string(virtualCode) + "    data3: " + to_string(data3) + "\n").c_str());
	}
	if (nType == IE_ButtonPressed) {
		if ((key == jumpBinds[0] || key == jumpBinds[1]) && !jumpPressHolder.waitingToSend) {
			if (crouchPressHolder.waitingToSend) {
				crouchPressHolder.waitingToSend = false;
				auto crouchElapsed = std::chrono::steady_clock::now() - crouchPressHolder.timestamp;
				long long sinceCrouch = std::chrono::duration_cast<std::chrono::milliseconds>(crouchElapsed).count();
				m_sourceConsole->Print(("crouchkick: " + to_string(sinceCrouch) + "ms CROUCH IS EARLY\n").c_str());

				//playSound();
				hookedPostEvent(crouchPressHolder.a, crouchPressHolder.nType, crouchPressHolder.nTick,
					crouchPressHolder.scanCode, crouchPressHolder.virtualCode, crouchPressHolder.data3);
			}
			else {
				jumpPressHolder.a = a;
				jumpPressHolder.nType = nType;
				jumpPressHolder.nTick = nTick;
				jumpPressHolder.scanCode = scanCode;
				jumpPressHolder.virtualCode = virtualCode;
				jumpPressHolder.data3 = data3;

				jumpPressHolder.waitingToSend = true;
				jumpPressHolder.timestamp = std::chrono::steady_clock::now();

				return;
			}
		}
		if ((key == crouchBinds[0] || key == crouchBinds[1]) && !crouchPressHolder.waitingToSend) {
			if (jumpPressHolder.waitingToSend) {
				jumpPressHolder.waitingToSend = false;
				auto jumpElapsed = std::chrono::steady_clock::now() - jumpPressHolder.timestamp;
				long long sinceJump = std::chrono::duration_cast<std::chrono::milliseconds>(jumpElapsed).count();
				m_sourceConsole->Print(("crouchkick: " + to_string(sinceJump) + "ms JUMP IS EARLY\n").c_str());

				//playSound();
				hookedPostEvent(jumpPressHolder.a, jumpPressHolder.nType, jumpPressHolder.nTick,
					jumpPressHolder.scanCode, jumpPressHolder.virtualCode, jumpPressHolder.data3);
			}
			else {
				crouchPressHolder.a = a;
				crouchPressHolder.nType = nType;
				crouchPressHolder.nTick = nTick;
				crouchPressHolder.scanCode = scanCode;
				crouchPressHolder.virtualCode = virtualCode;
				crouchPressHolder.data3 = data3;

				crouchPressHolder.waitingToSend = true;
				crouchPressHolder.timestamp = std::chrono::steady_clock::now();

				return;
			}
		}
	}
	if (nType == IE_ButtonReleased) {
		if ((key == jumpBinds[0] || key == jumpBinds[1]) && !jumpReleaseHolder.waitingToSend) {
			jumpReleaseHolder.a = a;
			jumpReleaseHolder.nType = nType;
			jumpReleaseHolder.nTick = nTick;
			jumpReleaseHolder.scanCode = scanCode;
			jumpReleaseHolder.virtualCode = virtualCode;
			jumpReleaseHolder.data3 = data3;

			jumpReleaseHolder.waitingToSend = true;
			jumpReleaseHolder.timestamp = std::chrono::steady_clock::now();
			return;
		}
		if ((key == crouchBinds[0] || key == crouchBinds[1]) && !crouchReleaseHolder.waitingToSend) {
			crouchReleaseHolder.a = a;
			crouchReleaseHolder.nType = nType;
			crouchReleaseHolder.nTick = nTick;
			crouchReleaseHolder.scanCode = scanCode;
			crouchReleaseHolder.virtualCode = virtualCode;
			crouchReleaseHolder.data3 = data3;

			crouchReleaseHolder.waitingToSend = true;
			crouchReleaseHolder.timestamp = std::chrono::steady_clock::now();

			return;
		}
	}
	hookedPostEvent(a, nType, nTick, scanCode, virtualCode, data3);
}

bool xinputHookSet;
bool postEventHookSet;

void setInputHooks() {
	if (postEventHookSet && xinputHookSet) return;

	uintptr_t moduleBase = (uintptr_t)GetModuleHandleW(L"inputsystem.dll");

	if (!postEventHookSet) {
		POSTEVENT postEvent = POSTEVENT(moduleBase + 0x7EC0);
		DWORD postEventResult = MH_CreateHookEx(postEvent, &detourPostEvent, &hookedPostEvent);
		if (postEventResult != MH_OK) {
			m_sourceConsole->Print(("hook post failed " + to_string(postEventResult) + "\n").c_str());
		}
		else {
			postEventHookSet = true;
		}
	}

	if (!xinputHookSet) {
		DWORD xinputResult = MH_CreateHookApiEx(L"XInput1_3", "XInputGetState", &detourXInputGetState, &hookedXInputGetState);
		if (xinputResult != MH_OK) {
			m_sourceConsole->Print(("hook XInputGetState failed " + to_string(xinputResult) + "\n").c_str());
		}
		else {
			xinputHookSet = true;
		}
	}

	hookDirectXPresent();
}

bool hooksEnabled = false;

void enableInputHooks() {
	if (!xinputHookSet || !postEventHookSet) return;
	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
	{
		m_sourceConsole->Print("enable hooks failed");
	}
	else {
		hooksEnabled = true;
	}
}

void disableInputHooks() {
	if (!xinputHookSet || !postEventHookSet) return;
	if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK)
	{
		m_sourceConsole->Print("disabling hooks failed");
	}
	else {
		hooksEnabled = false;
	}
}

HRESULT __stdcall detourD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
	if (jumpPressHolder.waitingToSend) {
		auto jumpElapsed = std::chrono::steady_clock::now() - jumpPressHolder.timestamp;
		long long sinceJump = std::chrono::duration_cast<std::chrono::milliseconds>(jumpElapsed).count();

		if (sinceJump > CROUCHKICK_BUFFERING) {
			jumpPressHolder.waitingToSend = false;
			hookedPostEvent(jumpPressHolder.a, jumpPressHolder.nType, jumpPressHolder.nTick,
				jumpPressHolder.scanCode, jumpPressHolder.virtualCode, jumpPressHolder.data3);
			auto e = std::chrono::steady_clock::now() - crouchtime;
			long long s = std::chrono::duration_cast<std::chrono::milliseconds>(e).count();

			if (s < 100) {
				m_sourceConsole->Print(("not crouchkick: " + to_string(s) + "ms CROUCH IS EARLY\n").c_str());
			}

			jumptime = std::chrono::steady_clock::now();
		}
	}
	if (jumpReleaseHolder.waitingToSend) {
		auto jumpElapsed = std::chrono::steady_clock::now() - jumpReleaseHolder.timestamp;
		long long sinceJump = std::chrono::duration_cast<std::chrono::milliseconds>(jumpElapsed).count();

		if (sinceJump > CROUCHKICK_BUFFERING) {
			jumpReleaseHolder.waitingToSend = false;
			hookedPostEvent(jumpReleaseHolder.a, jumpReleaseHolder.nType, jumpReleaseHolder.nTick,
				jumpReleaseHolder.scanCode, jumpReleaseHolder.virtualCode, jumpReleaseHolder.data3);
		}
	}

	if (crouchPressHolder.waitingToSend) {
		auto crouchElapsed = std::chrono::steady_clock::now() - crouchPressHolder.timestamp;
		long long sinceCrouch = std::chrono::duration_cast<std::chrono::milliseconds>(crouchElapsed).count();

		if (sinceCrouch > CROUCHKICK_BUFFERING) {
			crouchPressHolder.waitingToSend = false;
			hookedPostEvent(crouchPressHolder.a, crouchPressHolder.nType, crouchPressHolder.nTick,
				crouchPressHolder.scanCode, crouchPressHolder.virtualCode, crouchPressHolder.data3);
			auto e = std::chrono::steady_clock::now() - jumptime;
			long long s = std::chrono::duration_cast<std::chrono::milliseconds>(e).count();

			if (s < 100) {
				m_sourceConsole->Print(("not crouchkick: " + to_string(s) + "ms JUMP IS EARLY\n").c_str());
			}

			crouchtime = std::chrono::steady_clock::now();
		}
	}
	if (crouchReleaseHolder.waitingToSend) {
		auto jumpElapsed = std::chrono::steady_clock::now() - crouchReleaseHolder.timestamp;
		long long sinceJump = std::chrono::duration_cast<std::chrono::milliseconds>(jumpElapsed).count();

		if (sinceJump > CROUCHKICK_BUFFERING) {
			crouchReleaseHolder.waitingToSend = false;
			hookedPostEvent(crouchReleaseHolder.a, crouchReleaseHolder.nType, crouchReleaseHolder.nTick,
				crouchReleaseHolder.scanCode, crouchReleaseHolder.virtualCode, crouchReleaseHolder.data3);
		}
	}
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