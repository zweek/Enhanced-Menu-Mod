#pragma once
#include <Windows.h>
#include <chrono>
#include <Xinput.h>
#include "TASTools.h"

struct InputHolder {
	__int64 a;
	HWND hWnd;
	UINT uMsg;
	WPARAM wParam;
	LPARAM lParam;

	bool waitingToSend;
	std::chrono::steady_clock::time_point timestamp;
};

struct ControllerInputHolder {
	int buttonIndex;

	bool waitingToSend;
	std::chrono::steady_clock::time_point timestamp;
};

enum SRMM_settings
{
	SRMM_ENABLE_SPEEDOMETER,
	SRMM_SPEEDOMETER_INCLUDE_Z,
	SRMM_SPEEDOMETER_FADEOUT,
	SRMM_TAS_MODE,
	SRMM_CK_FIX,
	SRMM_ENABLE_SPEEDMOD
};

const long long CROUCHKICK_BUFFERING = 8;

void hookedInputProc(__int64, HWND, UINT, WPARAM, LPARAM);

extern bool hooksEnabled;
extern bool allHooksSet;
extern bool consoleEnabled;

void simulateKeyDown(WPARAM key);
void simulateKeyUp(WPARAM key);
void setInputHooks();
void enableInputHooks();
void disableInputHooks();