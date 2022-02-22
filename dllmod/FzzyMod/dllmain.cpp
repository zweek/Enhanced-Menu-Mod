#include <Windows.h>
#include <chrono>
//#include <thread>
#include <iostream>
//#include <vector>
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

bool SRMM_GetSetting(int pos) {
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
	if (!SRMM_GetSetting(SRMM_SPEEDOMETER_INCLUDE_Z)) {
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
	if (!SRMM_GetSetting(SRMM_SPEEDOMETER_FADEOUT)) {
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

bool consoleCreated = false;

DWORD WINAPI Thread(HMODULE hModule) {
	Sleep(7000);
	//AllocConsole();
	//freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

	MH_Initialize();

	//ModAltTab();

	while (true) {
		Sleep(1000);

		setInputHooks();

		ModSpeedometer();

		if (!hooksEnabled && SRMM_GetSetting(SRMM_CK_FIX)) {
			enableInputHooks();
		}
		if (hooksEnabled && !SRMM_GetSetting(SRMM_CK_FIX)) {
			disableInputHooks();
		}

		if (SRMM_GetSetting(SRMM_ENABLE_CONSOLE) && !consoleCreated) {
			AllocConsole();
			freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
			consoleCreated = true;
		}
		if (!SRMM_GetSetting(SRMM_ENABLE_CONSOLE) && consoleCreated) {
			HWND console = GetConsoleWindow();
			FreeConsole();
			PostMessage(console, WM_CLOSE, 0, 0);
			consoleCreated = false;
		}

		findBinds();
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
