#include <Windows.h>
#include "TF2Binds.h"
#include <iostream>
#include "SourceConsole.h"
#include "InputHooker.h"

using namespace std;

int jumpBinds[2];
int crouchBinds[2];
int forwardBinds[2];
int backBinds[2];
int rightBinds[2];
int leftBinds[2];
int tasLurchBinds[2];
int tasBumpLaunchBinds[2];
int tasSurfStopBinds[2];

int controllerJump;
int controllerCrouch;

int s_pButtonCodeToVirtual[BUTTON_CODE_LAST];
int s_pVirtualKeyToButtonCode[256];

static ButtonCode_t s_pScanToButtonCode_QWERTY[128] =
{
	//	0				1				2				3				4				5				6				7 
	//	8				9				A				B				C				D				E				F 
	KEY_NONE,		KEY_ESCAPE,		KEY_1,			KEY_2,			KEY_3,			KEY_4,			KEY_5,			KEY_6,			// 0
	KEY_7,			KEY_8,			KEY_9,			KEY_0,			KEY_MINUS,		KEY_EQUAL,		KEY_BACKSPACE,	KEY_TAB,		// 0 

	KEY_Q,			KEY_W,			KEY_E,			KEY_R,			KEY_T,			KEY_Y,			KEY_U,			KEY_I,			// 1
	KEY_O,			KEY_P,			KEY_LBRACKET,	KEY_RBRACKET,	KEY_ENTER,		KEY_LCONTROL,	KEY_A,			KEY_S,			// 1 

	KEY_D,			KEY_F,			KEY_G,			KEY_H,			KEY_J,			KEY_K,			KEY_L,			KEY_SEMICOLON,	// 2 
	KEY_APOSTROPHE,	KEY_BACKQUOTE,	KEY_LSHIFT,		KEY_BACKSLASH,	KEY_Z,			KEY_X,			KEY_C,			KEY_V,			// 2 

	KEY_B,			KEY_N,			KEY_M,			KEY_COMMA,		KEY_PERIOD,		KEY_SLASH,		KEY_RSHIFT,		KEY_PAD_MULTIPLY,// 3
	KEY_LALT,		KEY_SPACE,		KEY_CAPSLOCK,	KEY_F1,			KEY_F2,			KEY_F3,			KEY_F4,			KEY_F5,			// 3 

	KEY_F6,			KEY_F7,			KEY_F8,			KEY_F9,			KEY_F10,		KEY_NUMLOCK,	KEY_SCROLLLOCK,	KEY_HOME,		// 4
	KEY_UP,			KEY_PAGEUP,		KEY_PAD_MINUS,	KEY_LEFT,		KEY_PAD_5,		KEY_RIGHT,		KEY_PAD_PLUS,	KEY_END,		// 4 

	KEY_DOWN,		KEY_PAGEDOWN,	KEY_INSERT,		KEY_DELETE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_F11,		// 5
	KEY_F12,		KEY_BREAK,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		// 5

	KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		// 6
	KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		// 6 

	KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		// 7
	KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE,		KEY_NONE		// 7 
};

int ButtonCode_VirtualKeyToScanCode(int virtualKey) {
	return ButtonCode_ButtonCodeToScanCode((ButtonCode_t)s_pVirtualKeyToButtonCode[virtualKey]);
}

int ButtonCode_ButtonCodeToScanCode(ButtonCode_t button) {
	for (int i = 0; i < sizeof(s_pScanToButtonCode_QWERTY); i++) {
		if (s_pScanToButtonCode_QWERTY[i] == button) return s_pScanToButtonCode_QWERTY[i];
	}
}

int filter(unsigned int code, struct _EXCEPTION_POINTERS* ep)
{
	return EXCEPTION_CONTINUE_EXECUTION;
}

ButtonCode_t ButtonCode_ScanCodeToButtonCode(int lParam)
{
	int nScanCode = (lParam >> 16) & 0xFF;
	if (nScanCode > 127)
		return KEY_NONE;

	ButtonCode_t result = s_pScanToButtonCode_QWERTY[nScanCode];

	bool bIsExtended = (lParam & (1 << 24)) != 0;
	if (!bIsExtended)
	{
		switch (result)
		{
		case KEY_HOME:
			return KEY_PAD_7;
		case KEY_UP:
			return KEY_PAD_8;
		case KEY_PAGEUP:
			return KEY_PAD_9;
		case KEY_LEFT:
			return KEY_PAD_4;
		case KEY_RIGHT:
			return KEY_PAD_6;
		case KEY_END:
			return KEY_PAD_1;
		case KEY_DOWN:
			return KEY_PAD_2;
		case KEY_PAGEDOWN:
			return KEY_PAD_3;
		case KEY_INSERT:
			return KEY_PAD_0;
		case KEY_DELETE:
			return KEY_PAD_DECIMAL;
		default:
			break;
		}
	}
	else
	{
		switch (result)
		{
		case KEY_ENTER:
			return KEY_PAD_ENTER;
		case KEY_LALT:
			return KEY_RALT;
		case KEY_LCONTROL:
			return KEY_RCONTROL;
		case KEY_SLASH:
			return KEY_PAD_DIVIDE;
		case KEY_CAPSLOCK:
			return KEY_PAD_PLUS;
		}
	}

	return result;
}

void InitializeTF2Binds() {
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
}

void findBind(char* bound, int& buttonCode, int* bind, int bindSize, int& index, char* search, int searchSize) {
	for (int i = 0; i < 100; i++) {
		if (bound[i] == '\0' && i == searchSize) {
			if (index >= bindSize) break;
			bind[index] = s_pButtonCodeToVirtual[buttonCode];
			index++;
			break;
		}
		if (i >= sizeof(search) || bound[i] != search[i]) break;
	}
}

void findBinds() {
	uintptr_t inputBase = (uintptr_t)GetModuleHandleW(L"inputsystem.dll");
	uintptr_t s_pButtonCodeNameAddr = inputBase + 0x61B90;
	if (!IsMemoryReadable(s_pButtonCodeNameAddr)) return;
	char** s_pButtonCodeName = (char**)(s_pButtonCodeNameAddr);

	uintptr_t engineBase = (uintptr_t)GetModuleHandleW(L"engine.dll");
	uintptr_t bindBase = 0x1396C5C0;

	char jumpSearch[] = "+ability 3";
	int jumpIndex = 0;

	char crouchSearch[] = "+duck";
	int crouchIndex = 0;

	char forwardSearch[] = "+forward";
	int forwardIndex = 0;

	char backSearch[] = "+back";
	int backIndex = 0;

	char leftSearch[] = "+moveleft";
	int leftIndex = 0;

	char rightSearch[] = "+moveright";
	int rightIndex = 0;

	char tasLurchSearch[] = "+taslurch";
	int tasLurchIndex = 0;

	char tasBumpLaunchSearch[] = "+tasbumplaunch";
	int tasBumpLaunchIndex = 0;

	char tasSurfStopSearch[] = "+tassurfstop";
	int tasSurfStopIndex = 0;

	jumpBinds[0] = 0;
	jumpBinds[1] = 0;
	crouchBinds[0] = 0;
	crouchBinds[1] = 0;
	forwardBinds[0] = 0;
	forwardBinds[1] = 0;
	backBinds[0] = 0;
	backBinds[1] = 0;
	leftBinds[0] = 0;
	leftBinds[1] = 0;
	rightBinds[0] = 0;
	rightBinds[1] = 0;
	tasLurchBinds[0] = 0;
	tasLurchBinds[1] = 0;
	tasBumpLaunchBinds[0] = 0;
	tasBumpLaunchBinds[1] = 0;
	tasSurfStopBinds[0] = 0;
	tasSurfStopBinds[1] = 0;

	for (int buttonCode = 0; buttonCode < BUTTON_CODE_COUNT; buttonCode++) {
		__try {
			int offset = buttonCode * 0x10;
			uintptr_t ptr = *reinterpret_cast<uintptr_t*>(engineBase + bindBase + offset);
			if (!IsMemoryReadable(ptr)) continue;
			if (ptr == 0) continue;

			char* bound = (char*)(ptr);

			//findBind(bound, buttonCode, crouchBinds, crouchSearch, crouchIndex);

			//findBind(bound, buttonCode, jumpBinds, sizeof(jumpBinds), jumpIndex, jumpSearch, sizeof(jumpSearch));

			// find jump binds
			for (int i = 0; i < 100; i++) {
				if (bound[i] == '\0' && i == sizeof(jumpSearch)) {
					if (jumpIndex >= sizeof(jumpBinds)) break;
					jumpBinds[jumpIndex] = buttonCode;
					jumpIndex++;
					break;
				}
				if (i >= sizeof(jumpSearch) || bound[i] != jumpSearch[i]) break;
			}

			// find crouch binds
			for (int i = 0; i < 100; i++) {
				if (bound[i] == '\0' && i == sizeof(crouchSearch)) {
					if (crouchIndex >= sizeof(crouchBinds)) break;
					crouchBinds[crouchIndex] = buttonCode;
					crouchIndex++;
					break;
				}
				if (i >= sizeof(crouchSearch) || bound[i] != crouchSearch[i]) break;
			}

			// find movement binds
			for (int i = 0; i < 100; i++) {
				if (bound[i] == '\0' && i == sizeof(forwardSearch)) {
					if (forwardIndex >= sizeof(forwardBinds)) break;
					forwardBinds[forwardIndex] = buttonCode;
					forwardIndex++;
					break;
				}
				if (i >= sizeof(forwardSearch) || bound[i] != forwardSearch[i]) break;
			}
			for (int i = 0; i < 100; i++) {
				if (bound[i] == '\0' && i == sizeof(backSearch)) {
					if (backIndex >= sizeof(backBinds)) break;
					backBinds[backIndex] = buttonCode;
					backIndex++;
					break;
				}
				if (i >= sizeof(backSearch) || bound[i] != backSearch[i]) break;
			}
			for (int i = 0; i < 100; i++) {
				if (bound[i] == '\0' && i == sizeof(leftSearch)) {
					if (leftIndex >= sizeof(leftBinds)) break;
					leftBinds[leftIndex] = buttonCode;
					leftIndex++;
					break;
				}
				if (i >= sizeof(leftSearch) || bound[i] != leftSearch[i]) break;
			}
			for (int i = 0; i < 100; i++) {
				if (bound[i] == '\0' && i == sizeof(rightSearch)) {
					if (rightIndex >= sizeof(rightBinds)) break;
					rightBinds[forwardIndex] = buttonCode;
					rightIndex++;
					break;
				}
				if (i >= sizeof(rightSearch) || bound[i] != rightSearch[i]) break;
			}
			for (int i = 0; i < 100; i++) {
				if (bound[i] == '\0' && i == sizeof(tasLurchSearch)) {
					if (tasLurchIndex >= sizeof(tasLurchBinds)) break;
					tasLurchBinds[forwardIndex] = buttonCode;
					tasLurchIndex++;
					break;
				}
				if (i >= sizeof(tasLurchSearch) || bound[i] != tasLurchSearch[i]) break;
			}
			for (int i = 0; i < 100; i++) {
				if (bound[i] == '\0' && i == sizeof(tasBumpLaunchSearch)) {
					if (tasBumpLaunchIndex >= sizeof(tasBumpLaunchBinds)) break;
					tasBumpLaunchBinds[forwardIndex] = buttonCode;
					tasBumpLaunchIndex++;
					break;
				}
				if (i >= sizeof(tasBumpLaunchSearch) || bound[i] != tasBumpLaunchSearch[i]) break;
			}
			for (int i = 0; i < 100; i++) {
				if (bound[i] == '\0' && i == sizeof(tasSurfStopSearch)) {
					if (tasSurfStopIndex >= sizeof(tasSurfStopBinds)) break;
					tasSurfStopBinds[forwardIndex] = buttonCode;
					tasSurfStopIndex++;
					break;
				}
				if (i >= sizeof(tasSurfStopSearch) || bound[i] != tasSurfStopSearch[i]) break;
			}
		}
		__except (filter(GetExceptionCode(), GetExceptionInformation())) {
		}
	}

	controllerJump = 0;
	controllerCrouch = 0;
	char controllerJumpSearch[] = "+ability 4";
	char controllerCrouchSearch[] = "+toggle_duck";

	for (int controllerCode = JOYSTICK_FIRST_BUTTON; controllerCode < KEY_XSTICK2_UP; controllerCode++) {
		__try {
			int offset = controllerCode * 0x10;
			uintptr_t ptr = *reinterpret_cast<uintptr_t*>(engineBase + bindBase + offset);
			if (!IsMemoryReadable(ptr)) continue;
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
		__except (filter(GetExceptionCode(), GetExceptionInformation())) {
		}
	}
}