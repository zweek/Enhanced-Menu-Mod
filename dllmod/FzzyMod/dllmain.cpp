#include <Windows.h>

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

void ModSpeedometer() {
	// tasEnabled: "engine.dll"+14C7A700, "vstdlib.dll"+000720D0

	uintptr_t base = (uintptr_t)GetModuleHandle("ui(11).dll");
	uintptr_t speedometer = base + 0x6AA50;

	uintptr_t alwaysShow = speedometer + 0x12F;
	WriteBytes((void*)alwaysShow, 0x90, 2);
	
	uintptr_t position1 = speedometer + 0x7A;
	WriteBytes((void*)position1, 0x12, 1);
	uintptr_t position2 = speedometer + 0x8C;
	WriteBytes((void*)position2, 0x12, 1);
}

//void ModAltTab() {
//	uintptr_t base = (uintptr_t)GetModuleHandle("engine.dll");
//	uintptr_t target = base + 0x1C8C17;
//
//	WriteBytes((void*)target, 0x75, 1);
//}

DWORD WINAPI Thread(HMODULE hModule) {
	Sleep(10000);
	ModSpeedometer();
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			// get path to midimap.dll file
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
