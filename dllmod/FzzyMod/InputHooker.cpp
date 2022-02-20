#pragma comment(lib, "winmm.lib")
#include "InputHooker.h"
#include "include/MinHook.h"
#include <iostream>
#include <mutex>
#include <string>
#include <bitset>
#include <cmath>

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

//int tasLurch[2];

int jump[2];
int crouch[2];

int controllerJump;
int controllerCrouch;

int s_pButtonCodeToVirtual[BUTTON_CODE_LAST];
int s_pVirtualKeyToButtonCode[256];

void findBinds() {
	uintptr_t inputBase = (uintptr_t)GetModuleHandleW(L"inputsystem.dll");
	char** s_pButtonCodeName = (char**)(inputBase + 0x61B90);

	uintptr_t engineBase = (uintptr_t)GetModuleHandleW(L"engine.dll");
	uintptr_t bindBase = 0x1396C5C0;

	char jumpSearch[] = "+ability 3";
	int jumpIndex = 0;

	char crouchSearch[] = "+duck";
	int crouchIndex = 0;

	char tasLurchSearch[] = "tas_lurch";
	int tasLurchIndex = 0;

	jump[0] = 0;
	jump[1] = 0;
	crouch[0] = 0;
	crouch[1] = 0;
	//tasLurch[0] = 0;
	//tasLurch[1] = 0;

	for (int buttonCode = 0; buttonCode < BUTTON_CODE_COUNT; buttonCode++) {
		int offset = buttonCode * 0x10;
		uintptr_t ptr = *reinterpret_cast<uint64_t*>(engineBase + bindBase + offset);
		if (ptr == 0) continue;

		char* bound = (char*)(ptr);

		// find jump binds
		for (int i = 0; i < 100; i++) {
			if (bound[i] == '\0' && i == sizeof(jumpSearch)) {
				if (jumpIndex >= sizeof(jump)) break;
				jump[jumpIndex] = s_pButtonCodeToVirtual[buttonCode];
				jumpIndex++;
				break;
			}
			if (i >= sizeof(jumpSearch) || bound[i] != jumpSearch[i]) break;
		}

		// find crouch binds
		for (int i = 0; i < 100; i++) {
			if (bound[i] == '\0' && i == sizeof(crouchSearch)) {
				if (crouchIndex >= sizeof(crouch)) break;
				crouch[crouchIndex] = s_pButtonCodeToVirtual[buttonCode];
				crouchIndex++;
				break;
			}
			if (i >= sizeof(crouchSearch) || bound[i] != crouchSearch[i]) break;
		}

		// find tas lurch binds
		/*for (int i = 0; i < 100; i++) {
			if (bound[i] == '\0' && i == sizeof(tasLurchSearch)) {
				if (tasLurchIndex >= sizeof(tasLurch)) break;
				tasLurch[tasLurchIndex] = s_pButtonCodeToVirtual[buttonCode];
				tasLurchIndex++;
				break;
			}
			if (i >= sizeof(tasLurchSearch) || bound[i] != tasLurchSearch[i]) break;
		}*/
	}

	controllerJump = 0;
	controllerCrouch = 0;
	char controllerJumpSearch[] = "+ability 4";
	char controllerCrouchSearch[] = "+toggle_duck";

	for (int controllerCode = JOYSTICK_FIRST_BUTTON; controllerCode < KEY_XSTICK2_UP; controllerCode++) {
		int offset = controllerCode * 0x10;
		uintptr_t ptr = *reinterpret_cast<uint64_t*>(engineBase + bindBase + offset);
		if (ptr == 0) continue;

		char* bound = (char*)(ptr);
		for (int i = 0; i < 100; i++) {
			if (bound[i] == '\0' && i == sizeof(controllerJumpSearch)) {
				controllerJump = controllerCode;
				break;
			}
			if (i >= sizeof(controllerJumpSearch) || bound[i] != controllerJumpSearch[i]) break;
		}
		for (int i = 0; i < 100; i++) {
			if (bound[i] == '\0' && i == sizeof(controllerCrouchSearch)) {
				controllerCrouch = controllerCode;
				break;
			}
			if (i >= sizeof(controllerCrouchSearch) || bound[i] != controllerCrouchSearch[i]) break;
		}
	}
}

//void playSound() {
//	struct stat buffer;
//	std::string name = "crouchkick.wav";
//	if (stat(name.c_str(), &buffer) == 0) {
//		PlaySound("crouchkick.wav", NULL, SND_ASYNC);
//	}
//}

InputHolder jumpPressHolder;
InputHolder jumpReleaseHolder;
InputHolder crouchPressHolder;
InputHolder crouchReleaseHolder;

int icount = 0;
auto itime = std::chrono::high_resolution_clock::now();

LRESULT __fastcall detourInputSystemProc(__int64 a, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (jumpPressHolder.waitingToSend) {
		auto jumpElapsed = std::chrono::high_resolution_clock::now() - jumpPressHolder.timestamp;
		long long sinceJump = std::chrono::duration_cast<std::chrono::microseconds>(jumpElapsed).count();

		if (sinceJump > CROUCHKICK_BUFFERING) {
			jumpPressHolder.waitingToSend = false;
			hookedInputProc(jumpPressHolder.a, jumpPressHolder.hWnd, jumpPressHolder.uMsg, jumpPressHolder.wParam, jumpPressHolder.lParam);
		}
	}
	if (jumpReleaseHolder.waitingToSend) {
		auto jumpElapsed = std::chrono::high_resolution_clock::now() - jumpReleaseHolder.timestamp;
		long long sinceJump = std::chrono::duration_cast<std::chrono::microseconds>(jumpElapsed).count();

		if (sinceJump > CROUCHKICK_BUFFERING) {
			jumpReleaseHolder.waitingToSend = false;
			hookedInputProc(jumpReleaseHolder.a, jumpReleaseHolder.hWnd, jumpReleaseHolder.uMsg, jumpReleaseHolder.wParam, jumpReleaseHolder.lParam);
		}
	}

	if (crouchPressHolder.waitingToSend) {
		auto crouchElapsed = std::chrono::high_resolution_clock::now() - crouchPressHolder.timestamp;
		long long sinceCrouch = std::chrono::duration_cast<std::chrono::microseconds>(crouchElapsed).count();

		if (sinceCrouch > CROUCHKICK_BUFFERING) {
			crouchPressHolder.waitingToSend = false;
			hookedInputProc(crouchPressHolder.a, crouchPressHolder.hWnd, crouchPressHolder.uMsg, crouchPressHolder.wParam, crouchPressHolder.lParam);
		}
	}
	if (crouchReleaseHolder.waitingToSend) {
		auto jumpElapsed = std::chrono::high_resolution_clock::now() - crouchReleaseHolder.timestamp;
		long long sinceJump = std::chrono::duration_cast<std::chrono::microseconds>(jumpElapsed).count();

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
	if (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONDBLCLK) key = VK_RBUTTON;
	if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK) key = VK_LBUTTON;
	if (uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONDBLCLK) {
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
		if ((key == jump[0] || key == jump[1]) && !jumpReleaseHolder.waitingToSend) {
			jumpReleaseHolder.a = a;
			jumpReleaseHolder.hWnd = hWnd;
			jumpReleaseHolder.uMsg = uMsg;
			jumpReleaseHolder.wParam = wParam;
			jumpReleaseHolder.lParam = lParam;

			jumpReleaseHolder.waitingToSend = true;
			jumpReleaseHolder.timestamp = std::chrono::high_resolution_clock::now();

			uMsg = WM_NULL;
		}
		if ((key == crouch[0] || key == crouch[1]) && !crouchReleaseHolder.waitingToSend) {
			crouchReleaseHolder.a = a;
			crouchReleaseHolder.hWnd = hWnd;
			crouchReleaseHolder.uMsg = uMsg;
			crouchReleaseHolder.wParam = wParam;
			crouchReleaseHolder.lParam = lParam;

			crouchReleaseHolder.waitingToSend = true;
			crouchReleaseHolder.timestamp = std::chrono::high_resolution_clock::now();

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
			if ((key == jump[0] || key == jump[1]) && !jumpPressHolder.waitingToSend) {
				if (crouchPressHolder.waitingToSend) {
					crouchPressHolder.waitingToSend = false;
					auto crouchElapsed = std::chrono::high_resolution_clock::now() - crouchPressHolder.timestamp;
					long long sinceCrouch = std::chrono::duration_cast<std::chrono::microseconds>(crouchElapsed).count();
					cout << "crouchkick: " << sinceCrouch / 100.0 << endl;

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
					jumpPressHolder.timestamp = std::chrono::high_resolution_clock::now();

					uMsg = WM_NULL;
				}
			}
			if ((key == crouch[0] || key == crouch[1]) && !crouchPressHolder.waitingToSend) {
				if (jumpPressHolder.waitingToSend) {
					jumpPressHolder.waitingToSend = false;
					auto jumpElapsed = std::chrono::high_resolution_clock::now() - jumpPressHolder.timestamp;
					long long sinceJump = std::chrono::duration_cast<std::chrono::microseconds>(jumpElapsed).count();
					cout << "crouchkick: " << sinceJump / 100.0 << endl;

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
					crouchPressHolder.timestamp = std::chrono::high_resolution_clock::now();

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
			if (key == jump[0]) {
				if (bDown && jumpPressHolder.waitingToSend) nButtonMask = nButtonMask & ~(1 << i);
				if (!bDown && jumpReleaseHolder.waitingToSend) nButtonMask = nButtonMask | (1 << i);
			}
			if (key == jump[1]) {
				if (bDown && jumpPressHolder.waitingToSend) nButtonMask = nButtonMask & ~(1 << i);
				if (!bDown && jumpReleaseHolder.waitingToSend) nButtonMask = nButtonMask | (1 << i);
			}
			if (key == crouch[0]) {
				if (bDown && crouchPressHolder.waitingToSend) nButtonMask = nButtonMask & ~(1 << i);
				if (!bDown && crouchReleaseHolder.waitingToSend) nButtonMask = nButtonMask | (1 << i);
			}
			if (key == crouch[1]) {
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

auto flipTimestamp = std::chrono::high_resolution_clock::now();
bool flip;

DWORD WINAPI detourXInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	DWORD toReturn = hookedXInputGetState(dwUserIndex, pState);

	/*float velX = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C2C);
	float velY = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C30);

	float yaw = *(float*)((uintptr_t)GetModuleHandle("engine.dll") + 0x7B6668);

	float toDegrees = (180.0f / 3.14159265f);
	float toRadians = (3.14159265f / 180.0f);
	float magnitude = sqrt(pow(velX, 2) + pow(velY, 2));

	auto elapsed = std::chrono::high_resolution_clock::now() - flipTimestamp;
	long long since = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	if (since > 100 * 1000) {
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
	}*/
	//}

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
			auto crouchElapsed = std::chrono::high_resolution_clock::now() - controllerCrouchPressHolder.timestamp;
			long long sinceCrouch = std::chrono::duration_cast<std::chrono::microseconds>(crouchElapsed).count();
			cout << "crouchkick: " << sinceCrouch / 1000.0 << endl;
		}
		else {
			controllerJumpPressHolder.waitingToSend = true;
			controllerJumpPressHolder.timestamp = std::chrono::high_resolution_clock::now();
		}
	}
	// Jump Release
	if (controllerJumpWasPressed && !jumpDown) {
		controllerJumpReleaseHolder.waitingToSend = true;
		controllerJumpReleaseHolder.timestamp = std::chrono::high_resolution_clock::now();
	}

	// Crouch Press
	if (!controllerCrouchWasPressed && crouchDown) {
		if (controllerJumpPressHolder.waitingToSend) {
			controllerJumpPressHolder.waitingToSend = false;
			//playSound();
			auto jumpElapsed = std::chrono::high_resolution_clock::now() - controllerJumpPressHolder.timestamp;
			long long sinceJump = std::chrono::duration_cast<std::chrono::microseconds>(jumpElapsed).count();
			cout << "crouchkick: " << sinceJump / 1000.0 << endl;
		}
		else {
			controllerCrouchPressHolder.waitingToSend = true;
			controllerCrouchPressHolder.timestamp = std::chrono::high_resolution_clock::now();
		}
	}
	// Crouch Release
	if (controllerCrouchWasPressed && !crouchDown) {
		controllerCrouchReleaseHolder.waitingToSend = true;
		controllerCrouchReleaseHolder.timestamp = std::chrono::high_resolution_clock::now();
	}

	if (controllerJumpReleaseHolder.waitingToSend) {
		auto jumpElapsed = std::chrono::high_resolution_clock::now() - controllerJumpReleaseHolder.timestamp;
		long long sinceJump = std::chrono::duration_cast<std::chrono::microseconds>(jumpElapsed).count();
		if (sinceJump > CROUCHKICK_BUFFERING) {
			controllerJumpReleaseHolder.waitingToSend = false;
		}
		else {
			pState->Gamepad.wButtons = pState->Gamepad.wButtons | (1 << jumpButtonIndex);
		}
	}
	if (controllerCrouchReleaseHolder.waitingToSend) {
		auto crouchElapsed = std::chrono::high_resolution_clock::now() - controllerCrouchReleaseHolder.timestamp;
		long long sinceCrouch = std::chrono::duration_cast<std::chrono::microseconds>(crouchElapsed).count();
		if (sinceCrouch > CROUCHKICK_BUFFERING) {
			controllerCrouchReleaseHolder.waitingToSend = false;
		}
		else {
			pState->Gamepad.wButtons = pState->Gamepad.wButtons | (1 << crouchButtonIndex);
		}
	}

	if (controllerJumpPressHolder.waitingToSend) {
		auto jumpElapsed = std::chrono::high_resolution_clock::now() - controllerJumpPressHolder.timestamp;
		long long sinceJump = std::chrono::duration_cast<std::chrono::microseconds>(jumpElapsed).count();
		if (sinceJump > CROUCHKICK_BUFFERING) {
			controllerJumpPressHolder.waitingToSend = false;
		}
		else {
			pState->Gamepad.wButtons = pState->Gamepad.wButtons & ~(1 << jumpButtonIndex);
		}
	}
	if (controllerCrouchPressHolder.waitingToSend) {
		auto crouchElapsed = std::chrono::high_resolution_clock::now() - controllerCrouchPressHolder.timestamp;
		long long sinceCrouch = std::chrono::duration_cast<std::chrono::microseconds>(crouchElapsed).count();
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

bool allHooksSet;

void setInputHooks() {
	if (allHooksSet) return;
	s_pVirtualKeyToButtonCode['0'] = KEY_0;
	s_pVirtualKeyToButtonCode['1'] = KEY_1;
	s_pVirtualKeyToButtonCode['2'] = KEY_2;
	s_pVirtualKeyToButtonCode['3'] = KEY_3;
	s_pVirtualKeyToButtonCode['4'] = KEY_4;
	s_pVirtualKeyToButtonCode['5'] = KEY_5;
	s_pVirtualKeyToButtonCode['6'] = KEY_6;
	s_pVirtualKeyToButtonCode['7'] = KEY_7;
	s_pVirtualKeyToButtonCode['8'] = KEY_8;
	s_pVirtualKeyToButtonCode['9'] = KEY_9;
	s_pVirtualKeyToButtonCode['A'] = KEY_A;
	s_pVirtualKeyToButtonCode['B'] = KEY_B;
	s_pVirtualKeyToButtonCode['C'] = KEY_C;
	s_pVirtualKeyToButtonCode['D'] = KEY_D;
	s_pVirtualKeyToButtonCode['E'] = KEY_E;
	s_pVirtualKeyToButtonCode['F'] = KEY_F;
	s_pVirtualKeyToButtonCode['G'] = KEY_G;
	s_pVirtualKeyToButtonCode['H'] = KEY_H;
	s_pVirtualKeyToButtonCode['I'] = KEY_I;
	s_pVirtualKeyToButtonCode['J'] = KEY_J;
	s_pVirtualKeyToButtonCode['K'] = KEY_K;
	s_pVirtualKeyToButtonCode['L'] = KEY_L;
	s_pVirtualKeyToButtonCode['M'] = KEY_M;
	s_pVirtualKeyToButtonCode['N'] = KEY_N;
	s_pVirtualKeyToButtonCode['O'] = KEY_O;
	s_pVirtualKeyToButtonCode['P'] = KEY_P;
	s_pVirtualKeyToButtonCode['Q'] = KEY_Q;
	s_pVirtualKeyToButtonCode['R'] = KEY_R;
	s_pVirtualKeyToButtonCode['S'] = KEY_S;
	s_pVirtualKeyToButtonCode['T'] = KEY_T;
	s_pVirtualKeyToButtonCode['U'] = KEY_U;
	s_pVirtualKeyToButtonCode['V'] = KEY_V;
	s_pVirtualKeyToButtonCode['W'] = KEY_W;
	s_pVirtualKeyToButtonCode['X'] = KEY_X;
	s_pVirtualKeyToButtonCode['Y'] = KEY_Y;
	s_pVirtualKeyToButtonCode['Z'] = KEY_Z;
	s_pVirtualKeyToButtonCode[VK_NUMPAD0] = KEY_PAD_0;
	s_pVirtualKeyToButtonCode[VK_NUMPAD1] = KEY_PAD_1;
	s_pVirtualKeyToButtonCode[VK_NUMPAD2] = KEY_PAD_2;
	s_pVirtualKeyToButtonCode[VK_NUMPAD3] = KEY_PAD_3;
	s_pVirtualKeyToButtonCode[VK_NUMPAD4] = KEY_PAD_4;
	s_pVirtualKeyToButtonCode[VK_NUMPAD5] = KEY_PAD_5;
	s_pVirtualKeyToButtonCode[VK_NUMPAD6] = KEY_PAD_6;
	s_pVirtualKeyToButtonCode[VK_NUMPAD7] = KEY_PAD_7;
	s_pVirtualKeyToButtonCode[VK_NUMPAD8] = KEY_PAD_8;
	s_pVirtualKeyToButtonCode[VK_NUMPAD9] = KEY_PAD_9;
	s_pVirtualKeyToButtonCode[VK_DIVIDE] = KEY_PAD_DIVIDE;
	s_pVirtualKeyToButtonCode[VK_MULTIPLY] = KEY_PAD_MULTIPLY;
	s_pVirtualKeyToButtonCode[VK_SUBTRACT] = KEY_PAD_MINUS;
	s_pVirtualKeyToButtonCode[VK_ADD] = KEY_PAD_PLUS;
	s_pVirtualKeyToButtonCode[VK_RETURN] = KEY_PAD_ENTER;
	s_pVirtualKeyToButtonCode[VK_DECIMAL] = KEY_PAD_DECIMAL;
	s_pVirtualKeyToButtonCode[0xdb] = KEY_LBRACKET;
	s_pVirtualKeyToButtonCode[0xdd] = KEY_RBRACKET;
	s_pVirtualKeyToButtonCode[0xba] = KEY_SEMICOLON;
	s_pVirtualKeyToButtonCode[0xde] = KEY_APOSTROPHE;
	s_pVirtualKeyToButtonCode[0xc0] = KEY_BACKQUOTE;
	s_pVirtualKeyToButtonCode[0xbc] = KEY_COMMA;
	s_pVirtualKeyToButtonCode[0xbe] = KEY_PERIOD;
	s_pVirtualKeyToButtonCode[0xbf] = KEY_SLASH;
	s_pVirtualKeyToButtonCode[0xdc] = KEY_BACKSLASH;
	s_pVirtualKeyToButtonCode[0xbd] = KEY_MINUS;
	s_pVirtualKeyToButtonCode[0xbb] = KEY_EQUAL;
	s_pVirtualKeyToButtonCode[VK_RETURN] = KEY_ENTER;
	s_pVirtualKeyToButtonCode[VK_SPACE] = KEY_SPACE;
	s_pVirtualKeyToButtonCode[VK_BACK] = KEY_BACKSPACE;
	s_pVirtualKeyToButtonCode[VK_TAB] = KEY_TAB;
	s_pVirtualKeyToButtonCode[VK_CAPITAL] = KEY_CAPSLOCK;
	s_pVirtualKeyToButtonCode[VK_NUMLOCK] = KEY_NUMLOCK;
	s_pVirtualKeyToButtonCode[VK_ESCAPE] = KEY_ESCAPE;
	s_pVirtualKeyToButtonCode[VK_SCROLL] = KEY_SCROLLLOCK;
	s_pVirtualKeyToButtonCode[VK_INSERT] = KEY_INSERT;
	s_pVirtualKeyToButtonCode[VK_DELETE] = KEY_DELETE;
	s_pVirtualKeyToButtonCode[VK_HOME] = KEY_HOME;
	s_pVirtualKeyToButtonCode[VK_END] = KEY_END;
	s_pVirtualKeyToButtonCode[VK_PRIOR] = KEY_PAGEUP;
	s_pVirtualKeyToButtonCode[VK_NEXT] = KEY_PAGEDOWN;
	s_pVirtualKeyToButtonCode[VK_PAUSE] = KEY_BREAK;
	s_pVirtualKeyToButtonCode[VK_SHIFT] = KEY_RSHIFT;
	s_pVirtualKeyToButtonCode[VK_SHIFT] = KEY_LSHIFT;	// SHIFT -> left SHIFT
	s_pVirtualKeyToButtonCode[VK_MENU] = KEY_RALT;
	s_pVirtualKeyToButtonCode[VK_MENU] = KEY_LALT;		// ALT -> left ALT
	s_pVirtualKeyToButtonCode[VK_CONTROL] = KEY_RCONTROL;
	s_pVirtualKeyToButtonCode[VK_CONTROL] = KEY_LCONTROL;	// CTRL -> left CTRL
	s_pVirtualKeyToButtonCode[VK_LWIN] = KEY_LWIN;
	s_pVirtualKeyToButtonCode[VK_RWIN] = KEY_RWIN;
	s_pVirtualKeyToButtonCode[VK_APPS] = KEY_APP;
	s_pVirtualKeyToButtonCode[VK_UP] = KEY_UP;
	s_pVirtualKeyToButtonCode[VK_LEFT] = KEY_LEFT;
	s_pVirtualKeyToButtonCode[VK_DOWN] = KEY_DOWN;
	s_pVirtualKeyToButtonCode[VK_RIGHT] = KEY_RIGHT;
	s_pVirtualKeyToButtonCode[VK_F1] = KEY_F1;
	s_pVirtualKeyToButtonCode[VK_F2] = KEY_F2;
	s_pVirtualKeyToButtonCode[VK_F3] = KEY_F3;
	s_pVirtualKeyToButtonCode[VK_F4] = KEY_F4;
	s_pVirtualKeyToButtonCode[VK_F5] = KEY_F5;
	s_pVirtualKeyToButtonCode[VK_F6] = KEY_F6;
	s_pVirtualKeyToButtonCode[VK_F7] = KEY_F7;
	s_pVirtualKeyToButtonCode[VK_F8] = KEY_F8;
	s_pVirtualKeyToButtonCode[VK_F9] = KEY_F9;
	s_pVirtualKeyToButtonCode[VK_F10] = KEY_F10;
	s_pVirtualKeyToButtonCode[VK_F11] = KEY_F11;
	s_pVirtualKeyToButtonCode[VK_F12] = KEY_F12;
	s_pVirtualKeyToButtonCode[VK_LBUTTON] = MOUSE_LEFT;
	s_pVirtualKeyToButtonCode[VK_RBUTTON] = MOUSE_RIGHT;
	s_pVirtualKeyToButtonCode[VK_MBUTTON] = MOUSE_MIDDLE;
	s_pVirtualKeyToButtonCode[VK_XBUTTON1] = MOUSE_4;
	s_pVirtualKeyToButtonCode[VK_XBUTTON2] = MOUSE_5;

	for (int i = 0; i < KEY_COUNT + MOUSE_COUNT; i++)
	{
		s_pButtonCodeToVirtual[s_pVirtualKeyToButtonCode[i]] = i;
	}

	uintptr_t moduleBase = (uintptr_t)GetModuleHandleW(L"inputsystem.dll");
	allHooksSet = true;

	INPUTSYSTEMPROC inputSystemProc = INPUTSYSTEMPROC(moduleBase + 0x8B80);
	DWORD inputSystemProcResult = MH_CreateHookEx(inputSystemProc, &detourInputSystemProc, &hookedInputSystemProc);
	if (inputSystemProcResult != MH_OK && inputSystemProcResult != MH_ERROR_ALREADY_CREATED) {
		cout << "hook InputSystemProc failed" << inputSystemProcResult << endl;
		allHooksSet = false;
	}

	UPDATEMOUSEBUTTONSTATE updateMouseButtonState = UPDATEMOUSEBUTTONSTATE(moduleBase + 0x8A20);
	DWORD mouseStateResult = MH_CreateHookEx(updateMouseButtonState, &detourUpdateMouseButtonState, &hookedUpdateMouseButtonState);
	if (mouseStateResult != MH_OK && mouseStateResult != MH_ERROR_ALREADY_CREATED) {
		cout << "hook updateMouseButtonState failed" << mouseStateResult << endl;
		allHooksSet = false;
	}

	DWORD xinputResult = MH_CreateHookApiEx(L"XInput1_3", "XInputGetState", &detourXInputGetState, &hookedXInputGetState);
	if (xinputResult != MH_OK && xinputResult != MH_ERROR_ALREADY_CREATED) {
		cout << "hook XInputGetState failed: " << xinputResult << endl;
		allHooksSet = false;
	}
}

bool hooksEnabled = false;

void enableInputHooks() {
	if (!allHooksSet) return;
	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
	{
		std::cout << "enabling hooks failed" << std::endl;
	}
	else {
		hooksEnabled = true;
	}
}

void disableInputHooks() {
	if (!allHooksSet) return;
	if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK)
	{
		std::cout << "disabling hooks failed" << std::endl;
	}
	else {
		hooksEnabled = false;
	}
}