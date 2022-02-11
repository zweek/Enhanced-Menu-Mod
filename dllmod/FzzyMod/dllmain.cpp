#include <Windows.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include "InputHooker.h"
#include "include/MinHook.h"

#pragma region Proxy
struct midimap_dll {
	HMODULE dll;
	FARPROC oDriverProc;
	FARPROC omodMessage;
	FARPROC omodmCallback;
} midimap;

extern "C" {
	FARPROC PA = 0;
	int runASM();

	void fDriverProc() { PA = midimap.oDriverProc; runASM(); }
	void fmodMessage() { PA = midimap.omodMessage; runASM(); }
	void fmodmCallback() { PA = midimap.omodmCallback; runASM(); }
}

void setupFunctions() {
	midimap.oDriverProc = GetProcAddress(midimap.dll, "DriverProc");
	midimap.omodMessage = GetProcAddress(midimap.dll, "modMessage");
	midimap.omodmCallback = GetProcAddress(midimap.dll, "modmCallback");
}
#pragma endregion

void WriteBytes(void* ptr, int byte, int size) {
	DWORD curProtection;
	VirtualProtect(ptr, size, PAGE_EXECUTE_READWRITE, &curProtection);
	memset(ptr, byte, size);
	DWORD temp;
	VirtualProtect(ptr, size, curProtection, &temp);
}

uintptr_t FindAddress(uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	// runs through every offset of a pointer to get the address
	for (unsigned int i = 0; i < offsets.size(); ++i) {
		addr = *(uintptr_t*)addr;
		addr += offsets[i];
	}
	return addr;
}

bool GetSRMMsetting(int pos) {
	// voice_forcemicrecord convar
	uintptr_t srmmSettingBase = (uintptr_t)GetModuleHandle("engine.dll") + 0x8A159C;
	uintptr_t srmmSetting = *(uintptr_t*)srmmSettingBase;
	// check for a 1 in the binary of srmmSetting at pos
	return (srmmSetting & ((unsigned long long)1 << pos)) > 0;
}

void ModSpeedometer() {

	uintptr_t engineBase = (uintptr_t)GetModuleHandle("engine.dll") + 0x14C7A700;

	// voice_forcemicrecord ConVar
	uintptr_t srmmSettingBase = (uintptr_t)GetModuleHandle("engine.dll") + 0x8A159C;
	uintptr_t srmmSetting = *(uintptr_t*)srmmSettingBase;

	uintptr_t base = (uintptr_t)GetModuleHandle("ui(11).dll");
	uintptr_t speedometer = base + 0x6AA50;
	// fadeout when moving slowly
	uintptr_t alwaysShow = speedometer + 0x12F;
	// player positions on current and previous frame
	uintptr_t position1 = speedometer + 0x7A;
	uintptr_t position2 = speedometer + 0x8C;

	// Include/Exclude Z Axis
	// check if bit at pos 1 is 0
	if (!GetSRMMsetting(1)) {
		// overwrite to only include x&y axis
		WriteBytes((void*)position1, 0x12, 1);
		WriteBytes((void*)position2, 0x12, 1);
	}
	else {
		// overwrite back to default values
		WriteBytes((void*)position1, 0x10, 1);
		WriteBytes((void*)position2, 0x10, 1);
	}

	// Enable/Disable Fadeout
	// check if bit at pos 2 is 0
	if (!GetSRMMsetting(2)) {
		// disable fadeout
		WriteBytes((void*)alwaysShow, 0x90, 2);
	}
	else {
		// overwrite back to default values
		WriteBytes((void*)alwaysShow, 0x72, 1);
		WriteBytes((void*)(alwaysShow + 0x1), 0x2C, 1);
	}
}

void ModAltTab() {
	uintptr_t base = (uintptr_t)GetModuleHandle("engine.dll");
	uintptr_t target = base + 0x1C8C17;

	WriteBytes((void*)target, 0x75, 1);
}

DWORD WINAPI Thread(HMODULE hModule) {
	Sleep(10000);
	//AllocConsole();
	//freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

	MH_Initialize();
	setInputHooks();

	auto periodic = std::chrono::high_resolution_clock::now();

	while (true) {
		Sleep(1);

		long long sincePeriodic = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - periodic).count();
		if (sincePeriodic > 1000 * 1000) {
			periodic = std::chrono::high_resolution_clock::now();

			if (!hooksEnabled && GetSRMMsetting(4)) {
				enableInputHooks();
			}
			if (hooksEnabled && !GetSRMMsetting(4)) {
				disableInputHooks();
			}

			ModSpeedometer();
			findBinds();
		}

		if (jumpInputHolder.waitingToPress) {
			auto jumpElapsed = std::chrono::high_resolution_clock::now() - jumpInputHolder.timestamp;
			long long sinceJump = std::chrono::duration_cast<std::chrono::microseconds>(jumpElapsed).count();

			if (sinceJump > CROUCHKICK_BUFFERING) {
				jumpInputHolder.waitingToPress = false;
				hookedInputProc(jumpInputHolder.a, jumpInputHolder.hWnd, jumpInputHolder.uMsg, jumpInputHolder.wParam, jumpInputHolder.lParam);
			}
		}

		if (crouchInputHolder.waitingToPress) {
			auto crouchElapsed = std::chrono::high_resolution_clock::now() - crouchInputHolder.timestamp;
			long long sinceCrouch = std::chrono::duration_cast<std::chrono::microseconds>(crouchElapsed).count();

			if (sinceCrouch > CROUCHKICK_BUFFERING) {
				crouchInputHolder.waitingToPress = false;
				hookedInputProc(crouchInputHolder.a, crouchInputHolder.hWnd, crouchInputHolder.uMsg, crouchInputHolder.wParam, crouchInputHolder.lParam);
			}
		}
	}
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		char path[MAX_PATH];
		GetWindowsDirectory(path, sizeof(path));

		strcat_s(path, "\\System32\\midimap.dll");
		midimap.dll = LoadLibrary(path);
		setupFunctions();

		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Thread, hModule, 0, nullptr));

		break;
	case DLL_PROCESS_DETACH:
		FreeLibrary(midimap.dll);
		break;
	}
	return 1;
}
