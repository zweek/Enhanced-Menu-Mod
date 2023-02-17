//#pragma comment(lib, "winmm.lib")
#include "InputHooker.h"
#include <MinHook.h>
#include <iostream>
#include <mutex>
#include <string>
#include <bitset>
#include <cmath>
#include "TF2Binds.h"
#include "SourceConsole.h"
#include <d3d11.h>
#include <processthreadsapi.h>

using namespace std;

typedef DWORD(WINAPI* XINPUTGETSTATE)(DWORD, XINPUT_STATE*);
typedef unsigned int(__fastcall* POSTEVENT)(uintptr_t, InputEventType_t, int, ButtonCode_t, ButtonCode_t, int);
typedef HRESULT(__stdcall* D3D11PRESENT) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef void(__fastcall* UPDATE)();

static UPDATE hookedUpdate = nullptr;
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

//auto jumptime = std::chrono::steady_clock::now();
//auto crouchtime = std::chrono::steady_clock::now();

ControllerInputHolder controllerJumpPressHolder;
ControllerInputHolder controllerCrouchPressHolder;
ControllerInputHolder controllerJumpReleaseHolder;
ControllerInputHolder controllerCrouchReleaseHolder;

bool controllerJumpWasPressed;
bool controllerCrouchWasPressed;

DWORD WINAPI detourXInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	DWORD toReturn = hookedXInputGetState(dwUserIndex, pState);

	//if (SRMM_GetSetting(SRMM_TAS_MODE)) TASProcessXInput(pState);

	//pState->Gamepad.sThumbLX = (short)32767;
	//m_sourceConsole->Print(("LY IMMEDIATE: " + to_string(pState->Gamepad.sThumbLY) + "\n").c_str());

	if (SRMM_GetSetting(SRMM_CK_FIX)) {
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
				struct timespec ts;
				timespec_get(&ts, TIME_UTC);
				long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
				long sinceCrouch = real - controllerCrouchPressHolder.timestamp;

				m_sourceConsole->Print(("crouchkick: " + to_string(sinceCrouch) + "\n").c_str());
			}
			else {
				controllerJumpPressHolder.waitingToSend = true;
				struct timespec ts;
				timespec_get(&ts, TIME_UTC);
				long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
				controllerJumpPressHolder.timestamp = real;
			}
		}
		// Jump Release
		if (controllerJumpWasPressed && !jumpDown) {
			controllerJumpReleaseHolder.waitingToSend = true;
			struct timespec ts;
			timespec_get(&ts, TIME_UTC);
			long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
			controllerJumpReleaseHolder.timestamp = real;
		}

		// Crouch Press
		if (!controllerCrouchWasPressed && crouchDown) {
			if (controllerJumpPressHolder.waitingToSend) {
				controllerJumpPressHolder.waitingToSend = false;
				//playSound();
				struct timespec ts;
				timespec_get(&ts, TIME_UTC);
				long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
				long sinceJump = real - controllerJumpPressHolder.timestamp;

				m_sourceConsole->Print(("crouchkick: " + to_string(sinceJump) + "\n").c_str());
			}
			else {
				controllerCrouchPressHolder.waitingToSend = true;
				struct timespec ts;
				timespec_get(&ts, TIME_UTC);
				long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
				controllerCrouchPressHolder.timestamp = real;
			}
		}
		// Crouch Release
		if (controllerCrouchWasPressed && !crouchDown) {
			controllerCrouchReleaseHolder.waitingToSend = true;
			struct timespec ts;
			timespec_get(&ts, TIME_UTC);
			long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
			controllerCrouchReleaseHolder.timestamp = real;
		}

		if (controllerJumpReleaseHolder.waitingToSend) {
			struct timespec ts;
			timespec_get(&ts, TIME_UTC);
			long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
			long sinceJump = real - controllerJumpReleaseHolder.timestamp;

			if (sinceJump > CROUCHKICK_BUFFERING) {
				controllerJumpReleaseHolder.waitingToSend = false;
			}
			else {
				pState->Gamepad.wButtons = pState->Gamepad.wButtons | (1 << jumpButtonIndex);
			}
		}
		if (controllerCrouchReleaseHolder.waitingToSend) {
			struct timespec ts;
			timespec_get(&ts, TIME_UTC);
			long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
			long sinceCrouch = real - controllerCrouchReleaseHolder.timestamp;

			if (sinceCrouch > CROUCHKICK_BUFFERING) {
				controllerCrouchReleaseHolder.waitingToSend = false;
			}
			else {
				pState->Gamepad.wButtons = pState->Gamepad.wButtons | (1 << crouchButtonIndex);
			}
		}

		if (controllerJumpPressHolder.waitingToSend) {
			struct timespec ts;
			timespec_get(&ts, TIME_UTC);
			long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
			long sinceJump = real - controllerJumpPressHolder.timestamp;

			if (sinceJump > CROUCHKICK_BUFFERING) {
				controllerJumpPressHolder.waitingToSend = false;
			}
			else {
				pState->Gamepad.wButtons = pState->Gamepad.wButtons & ~(1 << jumpButtonIndex);
			}
		}
		if (controllerCrouchPressHolder.waitingToSend) {
			struct timespec ts;
			timespec_get(&ts, TIME_UTC);
			long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
			long sinceCrouch = real - controllerCrouchPressHolder.timestamp;

			if (sinceCrouch > CROUCHKICK_BUFFERING) {
				controllerCrouchPressHolder.waitingToSend = false;
			}
			else {
				pState->Gamepad.wButtons = pState->Gamepad.wButtons & ~(1 << crouchButtonIndex);
			}
		}

		controllerJumpWasPressed = jumpDown;
		controllerCrouchWasPressed = crouchDown;
	}

	//m_sourceConsole->Print(("LX END: " + to_string(pState->Gamepad.sThumbLX) + "\n").c_str());

	return toReturn;
}

void spoofPostEvent(InputEventType_t nType, int nTick, ButtonCode_t scanCode, ButtonCode_t virtualCode, int data3) {
	uintptr_t moduleBase = (uintptr_t)GetModuleHandleW(L"inputsystem.dll");
	hookedPostEvent(moduleBase + 0x69F40, nType, nTick, scanCode, virtualCode, data3);
}

unsigned int __fastcall detourPostEvent(uintptr_t a, InputEventType_t nType, int nTick, ButtonCode_t scanCode, ButtonCode_t virtualCode, int data3) {
	try {
		ButtonCode_t key = scanCode;
		//if (SRMM_GetSetting(SRMM_TAS_MODE) && TASProcessInput(a, nType, nTick, scanCode, virtualCode, data3)) return 0;
		if (SRMM_GetSetting(SRMM_CK_FIX)) {
			if (nType == IE_ButtonPressed) {
				if ((key == jumpBinds[0] || key == jumpBinds[1]) && !jumpPressHolder.waitingToSend) {
					struct timespec ts;
					timespec_get(&ts, TIME_UTC);
					long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
					long sinceCrouch = real - crouchPressHolder.timestamp;
					if (crouchPressHolder.waitingToSend && sinceCrouch <= CROUCHKICK_BUFFERING) {
						crouchPressHolder.waitingToSend = false;
						//auto crouchElapsed = std::chrono::steady_clock::now() - crouchPressHolder.timestamp;
						//long long sinceCrouch = std::chrono::duration_cast<std::chrono::milliseconds>(crouchElapsed).count();
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
						struct timespec ts;
						timespec_get(&ts, TIME_UTC);
						long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
						jumpPressHolder.timestamp = real;

						return 0;
					}
				}
				if ((key == crouchBinds[0] || key == crouchBinds[1]) && !crouchPressHolder.waitingToSend) {
					struct timespec ts;
					timespec_get(&ts, TIME_UTC);
					long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
					long sinceJump = real - jumpPressHolder.timestamp;
					if (jumpPressHolder.waitingToSend && sinceJump <= CROUCHKICK_BUFFERING) {
						jumpPressHolder.waitingToSend = false;

						//auto jumpElapsed = std::chrono::steady_clock::now() - jumpPressHolder.timestamp;
						//long long sinceJump = std::chrono::duration_cast<std::chrono::milliseconds>(jumpElapsed).count();
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
						struct timespec ts;
						timespec_get(&ts, TIME_UTC);
						long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
						crouchPressHolder.timestamp = real;

						return 0;
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
					struct timespec ts;
					timespec_get(&ts, TIME_UTC);
					long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
					jumpReleaseHolder.timestamp = real;

					return 0;
				}
				if ((key == crouchBinds[0] || key == crouchBinds[1]) && !crouchReleaseHolder.waitingToSend) {
					crouchReleaseHolder.a = a;
					crouchReleaseHolder.nType = nType;
					crouchReleaseHolder.nTick = nTick;
					crouchReleaseHolder.scanCode = scanCode;
					crouchReleaseHolder.virtualCode = virtualCode;
					crouchReleaseHolder.data3 = data3;

					crouchReleaseHolder.waitingToSend = true;
					struct timespec ts;
					timespec_get(&ts, TIME_UTC);
					long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
					crouchReleaseHolder.timestamp = real;

					return 0;
				}
			}
		}
	}
	catch (...) {}
	uintptr_t moduleBase = (uintptr_t)GetModuleHandleW(L"inputsystem.dll");
	unsigned int toReturn = hookedPostEvent(moduleBase + 0x69F40, nType, nTick, scanCode, virtualCode, data3);
	return toReturn;
}

bool half;
long long jumptime;
long long crouchtime;

void __fastcall detourUpdate() {
	hookedUpdate();
	try {

		half = !half;
		if (half) return;

		//if (SRMM_GetSetting(SRMM_TAS_MODE)) TASFrameHook();

		if (SRMM_GetSetting(SRMM_CK_FIX)) {
			if (jumpPressHolder.waitingToSend) {
				struct timespec ts;
				timespec_get(&ts, TIME_UTC);
				long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
				long sinceJump = real - jumpPressHolder.timestamp;

				if (sinceJump > CROUCHKICK_BUFFERING) {
					jumpPressHolder.waitingToSend = false;
					hookedPostEvent(jumpPressHolder.a, jumpPressHolder.nType, jumpPressHolder.nTick,
						jumpPressHolder.scanCode, jumpPressHolder.virtualCode, jumpPressHolder.data3);

					long long e = real - crouchtime;

					if (e < 100) {
						m_sourceConsole->Print(("not crouchkick: " + to_string(e) + "ms CROUCH IS EARLY\n").c_str());
					}

					jumptime = real;
				}
			}
			if (jumpReleaseHolder.waitingToSend) {
				struct timespec ts;
				timespec_get(&ts, TIME_UTC);
				long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
				long sinceJump = real - jumpReleaseHolder.timestamp;

				if (sinceJump > CROUCHKICK_BUFFERING) {
					jumpReleaseHolder.waitingToSend = false;
					hookedPostEvent(jumpReleaseHolder.a, jumpReleaseHolder.nType, jumpReleaseHolder.nTick,
						jumpReleaseHolder.scanCode, jumpReleaseHolder.virtualCode, jumpReleaseHolder.data3);
				}
			}

			if (crouchPressHolder.waitingToSend) {
				struct timespec ts;
				timespec_get(&ts, TIME_UTC);
				long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
				long sinceCrouch = real - crouchPressHolder.timestamp;

				if (sinceCrouch > CROUCHKICK_BUFFERING) {
					crouchPressHolder.waitingToSend = false;
					hookedPostEvent(crouchPressHolder.a, crouchPressHolder.nType, crouchPressHolder.nTick,
						crouchPressHolder.scanCode, crouchPressHolder.virtualCode, crouchPressHolder.data3);

					long long e = real - jumptime;

					if (e < 100) {
						m_sourceConsole->Print(("not crouchkick: " + to_string(e) + "ms CROUCH IS EARLY\n").c_str());
					}

					crouchtime = real;
				}
			}
			if (crouchReleaseHolder.waitingToSend) {
				struct timespec ts;
				timespec_get(&ts, TIME_UTC);
				long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
				long sinceCrouch = real - crouchReleaseHolder.timestamp;

				if (sinceCrouch > CROUCHKICK_BUFFERING) {
					crouchReleaseHolder.waitingToSend = false;
					hookedPostEvent(crouchReleaseHolder.a, crouchReleaseHolder.nType, crouchReleaseHolder.nTick,
						crouchReleaseHolder.scanCode, crouchReleaseHolder.virtualCode, crouchReleaseHolder.data3);
				}
			}
		}
	}
	catch (...) {}
}

void setInputHooks() {
	uintptr_t moduleBase = (uintptr_t)GetModuleHandleW(L"inputsystem.dll");

	POSTEVENT postEvent = POSTEVENT(moduleBase + 0x7EC0);
	DWORD postEventResult = MH_CreateHookEx(postEvent, &detourPostEvent, &hookedPostEvent);
	if (postEventResult != MH_OK) {
		m_sourceConsole->Print(("hook post failed " + to_string(postEventResult) + "\n").c_str());
	}

	/*DWORD xinputResult = MH_CreateHookApiEx(L"XInput1_3", "XInputGetState", &detourXInputGetState, &hookedXInputGetState);
	if (xinputResult != MH_OK) {
		m_sourceConsole->Print(("hook XInputGetState failed " + to_string(xinputResult) + "\n").c_str());
	}*/
	uintptr_t engineBase = (uintptr_t)GetModuleHandleW(L"engine.dll");

	UPDATE update = UPDATE(engineBase + 0x77F50);
	DWORD updateResult = MH_CreateHookEx(update, &detourUpdate, &hookedUpdate);

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
	{
		m_sourceConsole->Print("enable hooks failed");
	}
}

bool FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets, uintptr_t& addr)
{
	addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		addr = FindAddress(addr);

		if (addr == 0) {
			return false;
		}

		addr += offsets[i];
	}
	return true;
}

uintptr_t FindAddress(uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		addr = FindAddress(addr);

		if (addr == 0) {
			return 0;
		}

		addr += offsets[i];
	}
	return addr;
}

uintptr_t FindAddress(uintptr_t ptr)
{
	uintptr_t addr = ptr;

	if (IsMemoryReadable(ptr)) {
		return *(uintptr_t*)ptr;
	}
	else {
		return 0;
	}
}

bool IsMemoryReadable(const uintptr_t nAddress)
{
	static SYSTEM_INFO sysInfo;
	if (!sysInfo.dwPageSize)
		GetSystemInfo(&sysInfo);

	MEMORY_BASIC_INFORMATION memInfo;
	if (!VirtualQuery(reinterpret_cast<LPCVOID>(nAddress), &memInfo, sizeof(memInfo)))
		return false;

	return memInfo.State & MEM_COMMIT && !(memInfo.Protect & PAGE_NOACCESS);
}

bool SRMM_GetSetting(int pos) {
	// voice_forcemicrecord convar
	uintptr_t srmmSettingBase = (uintptr_t)GetModuleHandle("engine.dll") + 0x8A159C;
	uintptr_t srmmSetting = *(uintptr_t*)srmmSettingBase;
	// check for a 1 in the binary of srmmSetting at pos
	return (srmmSetting & ((unsigned long long)1 << pos)) > 0;
}