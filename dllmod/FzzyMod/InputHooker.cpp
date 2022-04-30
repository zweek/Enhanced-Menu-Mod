//#pragma comment(lib, "winmm.lib")
#include "InputHooker.h"
#include "include/MinHook.h"
#include <iostream>
#include <mutex>
#include <string>
#include <bitset>
#include <cmath>
#include "TF2Binds.h"

using namespace std;

typedef DWORD(WINAPI* XINPUTGETSTATE)(DWORD, XINPUT_STATE*);
typedef LRESULT(__fastcall* INPUTSYSTEMPROC)(__int64, HWND, UINT, WPARAM, LPARAM);
typedef void(__fastcall* UPDATEMOUSEBUTTONSTATE)(__int64, UINT, int);

static XINPUTGETSTATE hookedXInputGetState = nullptr;
static INPUTSYSTEMPROC hookedInputSystemProc = nullptr;
static UPDATEMOUSEBUTTONSTATE hookedUpdateMouseButtonState = nullptr;

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

bool gameClosing;

HWND lastWindow;

void simulateKeyDown(WPARAM key) {
	LPARAM lp;
	int scanCode = ButtonCode_VirtualKeyToScanCode(key);

	cout << "simulated scanCode: " << scanCode << endl;
	for (int i = 0; i < 8; i++) {
		int scan = (scanCode & (1 << i)) != 0;

		if (scan) {
			lp = lp | (1 << (i + 16));
		}
		else {
			lp = lp & ~(1 << (i + 16));
		}
	}

	SendMessage(lastWindow, WM_KEYDOWN, key, lp);
}

void simulateKeyUp(WPARAM key) {
	SendMessage(lastWindow, WM_KEYUP, key, NULL);
}

LRESULT __fastcall detourInputSystemProc(__int64 a, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	//lastWindow = hWnd;
	if (uMsg == WM_CLOSE) gameClosing = true;
	if (gameClosing) return hookedInputSystemProc(a, hWnd, uMsg, wParam, lParam);

	//TASProcessInputProc(uMsg, wParam, lParam);

	if (jumpPressHolder.waitingToSend) {
		auto jumpElapsed = std::chrono::steady_clock::now() - jumpPressHolder.timestamp;
		long long sinceJump = std::chrono::duration_cast<std::chrono::milliseconds>(jumpElapsed).count();

		if (sinceJump > CROUCHKICK_BUFFERING) {
			jumpPressHolder.waitingToSend = false;
			hookedInputProc(jumpPressHolder.a, jumpPressHolder.hWnd, jumpPressHolder.uMsg, jumpPressHolder.wParam, jumpPressHolder.lParam);
			auto e = std::chrono::steady_clock::now() - crouchtime;
			long long s = std::chrono::duration_cast<std::chrono::milliseconds>(e).count();

			if (s < 100) {
				if (consoleEnabled) cout << "not crouchkick: " << s << "ms CROUCH IS EARLY" << endl;
			}

			jumptime = std::chrono::steady_clock::now();
		}
	}
	if (jumpReleaseHolder.waitingToSend) {
		auto jumpElapsed = std::chrono::steady_clock::now() - jumpReleaseHolder.timestamp;
		long long sinceJump = std::chrono::duration_cast<std::chrono::milliseconds>(jumpElapsed).count();

		if (sinceJump > CROUCHKICK_BUFFERING) {
			jumpReleaseHolder.waitingToSend = false;
			hookedInputProc(jumpReleaseHolder.a, jumpReleaseHolder.hWnd, jumpReleaseHolder.uMsg, jumpReleaseHolder.wParam, jumpReleaseHolder.lParam);
		}
	}

	if (crouchPressHolder.waitingToSend) {
		auto crouchElapsed = std::chrono::steady_clock::now() - crouchPressHolder.timestamp;
		long long sinceCrouch = std::chrono::duration_cast<std::chrono::milliseconds>(crouchElapsed).count();

		if (sinceCrouch > CROUCHKICK_BUFFERING) {
			crouchPressHolder.waitingToSend = false;
			hookedInputProc(crouchPressHolder.a, crouchPressHolder.hWnd, crouchPressHolder.uMsg, crouchPressHolder.wParam, crouchPressHolder.lParam);
			auto e = std::chrono::steady_clock::now() - jumptime;
			long long s = std::chrono::duration_cast<std::chrono::milliseconds>(e).count();

			if (s < 100) {
				if (consoleEnabled) cout << "not crouchkick: " << s << "ms JUMP IS EARLY" << endl;
			}

			crouchtime = std::chrono::steady_clock::now();
		}
	}
	if (crouchReleaseHolder.waitingToSend) {
		auto jumpElapsed = std::chrono::steady_clock::now() - crouchReleaseHolder.timestamp;
		long long sinceJump = std::chrono::duration_cast<std::chrono::milliseconds>(jumpElapsed).count();

		if (sinceJump > CROUCHKICK_BUFFERING) {
			crouchReleaseHolder.waitingToSend = false;
			hookedInputProc(crouchReleaseHolder.a, crouchReleaseHolder.hWnd, crouchReleaseHolder.uMsg, crouchReleaseHolder.wParam, crouchReleaseHolder.lParam);
		}
	}

	int clFrames = *(int*)(*(uintptr_t*)((uintptr_t)GetModuleHandle("materialsystem_dx11.dll") + 0x1A9F4A8) + 0x58C);
	bool inLoadingScreen = *(bool*)((uintptr_t)GetModuleHandle("client.dll") + 0xB38C5C);
	int tickCount = *(int*)((uintptr_t)GetModuleHandle("engine.dll") + 0x765A24);
	int paused = *(int*)((uintptr_t)GetModuleHandle("engine.dll") + 0x12A53D48);
	if (clFrames <= 0 || inLoadingScreen || tickCount <= 23 || paused != 2) return hookedInputSystemProc(a, hWnd, uMsg, wParam, lParam);

	WPARAM key = wParam;
	if (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONDBLCLK || uMsg == WM_RBUTTONUP) key = VK_RBUTTON;
	if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK || uMsg == WM_LBUTTONUP) key = VK_LBUTTON;
	if (uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONDBLCLK || uMsg == WM_XBUTTONUP) {
		UINT button = GET_XBUTTON_WPARAM(wParam);
		if (button == XBUTTON1) {
			key = VK_XBUTTON1;
		}
		else {
			key = VK_XBUTTON2;
		}
	}
	switch (uMsg) {
	case WM_RBUTTONUP:
	case WM_LBUTTONUP:
	case WM_XBUTTONUP:
	case WM_KEYUP:
		if ((key == jumpBinds[0] || key == jumpBinds[1]) && !jumpReleaseHolder.waitingToSend) {
			jumpReleaseHolder.a = a;
			jumpReleaseHolder.hWnd = hWnd;
			jumpReleaseHolder.uMsg = uMsg;
			jumpReleaseHolder.wParam = wParam;
			jumpReleaseHolder.lParam = lParam;

			jumpReleaseHolder.waitingToSend = true;
			jumpReleaseHolder.timestamp = std::chrono::steady_clock::now();

			uMsg = WM_NULL;
		}
		if ((key == crouchBinds[0] || key == crouchBinds[1]) && !crouchReleaseHolder.waitingToSend) {
			crouchReleaseHolder.a = a;
			crouchReleaseHolder.hWnd = hWnd;
			crouchReleaseHolder.uMsg = uMsg;
			crouchReleaseHolder.wParam = wParam;
			crouchReleaseHolder.lParam = lParam;

			crouchReleaseHolder.waitingToSend = true;
			crouchReleaseHolder.timestamp = std::chrono::steady_clock::now();

			uMsg = WM_NULL;
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONDBLCLK:
	case WM_KEYDOWN:
		if (!(lParam & (1 << 30)))
		{
			if ((key == jumpBinds[0] || key == jumpBinds[1]) && !jumpPressHolder.waitingToSend) {
				if (crouchPressHolder.waitingToSend) {
					crouchPressHolder.waitingToSend = false;
					auto crouchElapsed = std::chrono::steady_clock::now() - crouchPressHolder.timestamp;
					long long sinceCrouch = std::chrono::duration_cast<std::chrono::milliseconds>(crouchElapsed).count();
					if (consoleEnabled) cout << "crouchkick: " << sinceCrouch << "ms CROUCH IS EARLY" << endl;

					//playSound();
					hookedInputProc(crouchPressHolder.a, crouchPressHolder.hWnd, crouchPressHolder.uMsg, crouchPressHolder.wParam, crouchPressHolder.lParam);
				}
				else {
					jumpPressHolder.a = a;
					jumpPressHolder.hWnd = hWnd;
					jumpPressHolder.uMsg = uMsg;
					jumpPressHolder.wParam = wParam;
					jumpPressHolder.lParam = lParam;

					jumpPressHolder.waitingToSend = true;
					jumpPressHolder.timestamp = std::chrono::steady_clock::now();

					uMsg = WM_NULL;
				}
			}
			if ((key == crouchBinds[0] || key == crouchBinds[1]) && !crouchPressHolder.waitingToSend) {
				if (jumpPressHolder.waitingToSend) {
					jumpPressHolder.waitingToSend = false;
					auto jumpElapsed = std::chrono::steady_clock::now() - jumpPressHolder.timestamp;
					long long sinceJump = std::chrono::duration_cast<std::chrono::milliseconds>(jumpElapsed).count();
					if (consoleEnabled) cout << "crouchkick: " << sinceJump << "ms JUMP IS EARLY" << endl;

					//playSound();
					hookedInputProc(jumpPressHolder.a, jumpPressHolder.hWnd, jumpPressHolder.uMsg, jumpPressHolder.wParam, jumpPressHolder.lParam);
				}
				else {
					crouchPressHolder.a = a;
					crouchPressHolder.hWnd = hWnd;
					crouchPressHolder.uMsg = uMsg;
					crouchPressHolder.wParam = wParam;
					crouchPressHolder.lParam = lParam;

					crouchPressHolder.waitingToSend = true;
					crouchPressHolder.timestamp = std::chrono::steady_clock::now();

					uMsg = WM_NULL;
				}
			}
		}
		break;
	}

	return hookedInputSystemProc(a, hWnd, uMsg, wParam, lParam);
}

void __fastcall detourUpdateMouseButtonState(__int64 a, UINT nButtonMask, int dblClickCode) {
	for (int i = 0; i < 5; ++i)
	{
		bool bDown = (nButtonMask & (1 << i)) != 0;
		INT key = VK_RBUTTON;
		switch (i) {
		case 0:
			key = VK_LBUTTON;
			break;
		case 1:
			key = VK_RBUTTON;
			break;
		case 2:
			key = VK_MBUTTON;
			break;
		case 3:
			key = VK_XBUTTON1;
			break;
		case 4:
			key = VK_XBUTTON2;
			break;
		}
		if (i == 0 || i == 1 || i == 2 || i == 3 || i == 4) {
			if (key == jumpBinds[0]) {
				if (bDown && jumpPressHolder.waitingToSend) nButtonMask = nButtonMask & ~(1 << i);
				if (!bDown && jumpReleaseHolder.waitingToSend) nButtonMask = nButtonMask | (1 << i);
			}
			if (key == jumpBinds[1]) {
				if (bDown && jumpPressHolder.waitingToSend) nButtonMask = nButtonMask & ~(1 << i);
				if (!bDown && jumpReleaseHolder.waitingToSend) nButtonMask = nButtonMask | (1 << i);
			}
			if (key == crouchBinds[0]) {
				if (bDown && crouchPressHolder.waitingToSend) nButtonMask = nButtonMask & ~(1 << i);
				if (!bDown && crouchReleaseHolder.waitingToSend) nButtonMask = nButtonMask | (1 << i);
			}
			if (key == crouchBinds[1]) {
				if (bDown && crouchPressHolder.waitingToSend) nButtonMask = nButtonMask & ~(1 << i);
				if (!bDown && crouchReleaseHolder.waitingToSend) nButtonMask = nButtonMask | (1 << i);
			}
		}
	}
	return hookedUpdateMouseButtonState(a, nButtonMask, dblClickCode);
}

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
			if (consoleEnabled) cout << "crouchkick: " << sinceCrouch << endl;
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
			if (consoleEnabled) cout << "crouchkick: " << sinceJump << endl;
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

	return toReturn;
}

void hookedInputProc(__int64 a, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	hookedInputSystemProc(a, hWnd, uMsg, wParam, lParam);
}

bool mouseHookSet;
bool xinputHookSet;
bool wndProcHookSet;

void setInputHooks() {
	if (mouseHookSet && xinputHookSet && wndProcHookSet) return;

	uintptr_t moduleBase = (uintptr_t)GetModuleHandleW(L"inputsystem.dll");

	if (!wndProcHookSet) {
		INPUTSYSTEMPROC inputSystemProc = INPUTSYSTEMPROC(moduleBase + 0x8B80);
		DWORD inputSystemProcResult = MH_CreateHookEx(inputSystemProc, &detourInputSystemProc, &hookedInputSystemProc);
		if (inputSystemProcResult != MH_OK) {
			if (consoleEnabled) cout << "hook InputSystemProc failed" << inputSystemProcResult << endl;
		}
		else {
			wndProcHookSet = true;
		}
	}

	if (!mouseHookSet) {
		UPDATEMOUSEBUTTONSTATE updateMouseButtonState = UPDATEMOUSEBUTTONSTATE(moduleBase + 0x8A20);
		DWORD mouseStateResult = MH_CreateHookEx(updateMouseButtonState, &detourUpdateMouseButtonState, &hookedUpdateMouseButtonState);
		if (mouseStateResult != MH_OK) {
			if (consoleEnabled) cout << "hook updateMouseButtonState failed" << mouseStateResult << endl;
		}
		else {
			mouseHookSet = true;
		}
	}

	if (!xinputHookSet) {
		DWORD xinputResult = MH_CreateHookApiEx(L"XInput1_3", "XInputGetState", &detourXInputGetState, &hookedXInputGetState);
		if (xinputResult != MH_OK) {
			if (consoleEnabled) cout << "hook XInputGetState failed: " << xinputResult << endl;
		}
		else {
			xinputHookSet = true;
		}
	}
}

bool hooksEnabled = false;

void enableInputHooks() {
	if (!mouseHookSet || !xinputHookSet || !wndProcHookSet) return;
	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
	{
		if (consoleEnabled) std::cout << "enabling hooks failed" << std::endl;
	}
	else {
		hooksEnabled = true;
	}
}

void disableInputHooks() {
	if (!mouseHookSet || !xinputHookSet || !wndProcHookSet) return;
	if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK)
	{
		if (consoleEnabled) std::cout << "disabling hooks failed" << std::endl;
	}
	else {
		hooksEnabled = false;
	}
}