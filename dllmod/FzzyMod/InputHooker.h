#pragma once
#include <Windows.h>
#include <chrono>
#include <Xinput.h>
#include "TF2Binds.h"

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
};

enum InputEventType_t
{
	IE_ButtonPressed = 0,	// m_nData contains a ButtonCode_t
	IE_ButtonReleased,		// m_nData contains a ButtonCode_t
	IE_ButtonDoubleClicked,	// m_nData contains a ButtonCode_t
	IE_AnalogValueChanged,	// m_nData contains an AnalogCode_t, m_nData2 contains the value

	IE_FirstSystemEvent = 100,
	IE_Quit = IE_FirstSystemEvent,
	IE_ControllerInserted,	// m_nData contains the controller ID
	IE_ControllerUnplugged,	// m_nData contains the controller ID

	IE_FirstVguiEvent = 1000,	// Assign ranges for other systems that post user events here
	IE_FirstAppEvent = 2000,
};

struct InputHolder {
	__int64 a;
	InputEventType_t nType;
	int nTick;
	ButtonCode_t scanCode;
	ButtonCode_t virtualCode;
	int data3;

	bool waitingToSend;
	std::chrono::steady_clock::time_point timestamp;
};

struct handle_data {
	unsigned long process_id;
	HWND best_handle;
};

const long long CROUCHKICK_BUFFERING = 8;

extern bool hooksEnabled;
extern bool allHooksSet;

void spoofPostEvent(__int64, InputEventType_t, int, ButtonCode_t, ButtonCode_t, int);

void setInputHooks();
void enableInputHooks();
void disableInputHooks();