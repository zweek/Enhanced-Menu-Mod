#pragma comment(lib, "winmm.lib")
#include "InputHooker.h"
#include "include/MinHook.h"
#include <iostream>
#include <mutex>
#include <string>

using namespace std;

typedef LRESULT(__fastcall* INPUTSYSTEMPROC)(__int64, HWND, UINT, WPARAM, LPARAM);
typedef void(__fastcall* UPDATEMOUSEBUTTONSTATE)(__int64, UINT, int);

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

InputHolder jumpInputHolder;
InputHolder crouchInputHolder;

int jump[2];
int crouch[2];

int s_pButtonCodeToVirtual[KEY_COUNT + MOUSE_COUNT];
int s_pVirtualKeyToButtonCode[256];

void findBinds() {
	uintptr_t inputBase = (uintptr_t)GetModuleHandleW(L"inputsystem.dll");
	char** s_pButtonCodeName = (char**)(inputBase + 0x61B90);

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

	uintptr_t engineBase = (uintptr_t)GetModuleHandleW(L"engine.dll");
	uintptr_t bindBase = 0x1396C5C0;

	char jumpSearch[] = "+ability 3";
	int jumpIndex = 0;

	char crouchSearch[] = "+duck";
	int crouchIndex = 0;

	jump[0] = 0;
	jump[1] = 0;
	crouch[0] = 0;
	crouch[1] = 0;

	for (int buttonCode = 0; buttonCode < KEY_COUNT + MOUSE_COUNT; buttonCode++) {
		int offset = buttonCode * 0x10;
		uintptr_t ptr = *reinterpret_cast<uint64_t*>(engineBase + bindBase + offset);
		if (ptr == 0) continue;

		char* bound = (char*)(ptr);
		for (int i = 0; i < 100; i++) {
			if (bound[i] == '\0' && i == sizeof(jumpSearch)) {
				if (jumpIndex >= sizeof(jump)) break;
				jump[jumpIndex] = s_pButtonCodeToVirtual[buttonCode];
				jumpIndex++;
				break;
			}
			if (i >= sizeof(jumpSearch) || bound[i] != jumpSearch[i]) break;
		}
		for (int i = 0; i < 100; i++) {
			if (bound[i] == '\0' && i == sizeof(crouchSearch)) {
				if (crouchIndex >= sizeof(crouch)) break;
				crouch[crouchIndex] = s_pButtonCodeToVirtual[buttonCode];
				crouchIndex++;
				break;
			}
			if (i >= sizeof(crouchSearch) || bound[i] != crouchSearch[i]) break;
		}
	}
}

void playSound() {
	struct stat buffer;
	std::string name = "crouchkick.wav";
	if (stat(name.c_str(), &buffer) == 0) {
		PlaySound("crouchkick.wav", NULL, SND_ASYNC);
	}
}

LRESULT __fastcall detourInputSystemProc(__int64 a, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONDBLCLK:
	case WM_KEYDOWN:
		if ((key == jump[0] || key == jump[1]) && !jumpInputHolder.waitingToPress) {
			if (crouchInputHolder.waitingToPress) {
				crouchInputHolder.waitingToPress = false;
				auto crouchElapsed = std::chrono::high_resolution_clock::now() - crouchInputHolder.timestamp;
				long long sinceCrouch = std::chrono::duration_cast<std::chrono::microseconds>(crouchElapsed).count();
				cout << "crouchkick: " << sinceCrouch / 1000.0 << endl;
				
				playSound();
				hookedInputProc(crouchInputHolder.a, crouchInputHolder.hWnd, crouchInputHolder.uMsg, crouchInputHolder.wParam, crouchInputHolder.lParam);
			}
			else {
				jumpInputHolder.a = a;
				jumpInputHolder.hWnd = hWnd;
				jumpInputHolder.uMsg = uMsg;
				jumpInputHolder.wParam = wParam;
				jumpInputHolder.lParam = lParam;

				jumpInputHolder.waitingToPress = true;
				jumpInputHolder.timestamp = std::chrono::high_resolution_clock::now();

				uMsg = WM_NULL;
			}
		}
		if ((key == crouch[0] || key == crouch[1]) && !crouchInputHolder.waitingToPress) {
			if (jumpInputHolder.waitingToPress) {
				jumpInputHolder.waitingToPress = false;
				auto jumpElapsed = std::chrono::high_resolution_clock::now() - jumpInputHolder.timestamp;
				long long sinceJump = std::chrono::duration_cast<std::chrono::microseconds>(jumpElapsed).count();
				cout << "crouchkick: " << sinceJump / 1000.0 << endl;

				playSound();
				hookedInputProc(jumpInputHolder.a, jumpInputHolder.hWnd, jumpInputHolder.uMsg, jumpInputHolder.wParam, jumpInputHolder.lParam);
			}
			else {
				crouchInputHolder.a = a;
				crouchInputHolder.hWnd = hWnd;
				crouchInputHolder.uMsg = uMsg;
				crouchInputHolder.wParam = wParam;
				crouchInputHolder.lParam = lParam;

				crouchInputHolder.waitingToPress = true;
				crouchInputHolder.timestamp = std::chrono::high_resolution_clock::now();

				uMsg = WM_NULL;
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
			if (key == jump[0] && bDown) {
				if (jumpInputHolder.waitingToPress) nButtonMask = nButtonMask & ~(1 << i);
			}
			if (key == jump[1] && bDown) {
				if (jumpInputHolder.waitingToPress) nButtonMask = nButtonMask & ~(1 << i);
			}
			if (key == crouch[0] && bDown) {
				if (crouchInputHolder.waitingToPress) nButtonMask = nButtonMask & ~(1 << i);
			}
			if (key == crouch[1] && bDown) {
				if (crouchInputHolder.waitingToPress) nButtonMask = nButtonMask & ~(1 << i);
			}
		}
	}
	return hookedUpdateMouseButtonState(a, nButtonMask, dblClickCode);
}

void hookedInputProc(__int64 a, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	hookedInputSystemProc(a, hWnd, uMsg, wParam, lParam);
}

void setInputHooks() {
	uintptr_t moduleBase = (uintptr_t)GetModuleHandleW(L"inputsystem.dll");

	INPUTSYSTEMPROC inputSystemProc = INPUTSYSTEMPROC(moduleBase + 0x8B80);
	if (MH_CreateHookEx(inputSystemProc, &detourInputSystemProc, &hookedInputSystemProc) != MH_OK) {
		cout << "hook InputSystemProc failed" << endl;
	}

	UPDATEMOUSEBUTTONSTATE updateMouseButtonState = UPDATEMOUSEBUTTONSTATE(moduleBase + 0x8A20);
	if (MH_CreateHookEx(updateMouseButtonState, &detourUpdateMouseButtonState, &hookedUpdateMouseButtonState) != MH_OK) {
		cout << "hook updateMouseButtonState failed" << endl;
	}
}

void enableInputHooks() {
	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
	{
		std::cout << "enabling hooks failed" << std::endl;
	}
}

void disableInputHooks() {
	MH_DisableHook(MH_ALL_HOOKS);
}