#pragma once
#include <Xinput.h>
#include "InputHooker.h"
#include "TF2Binds.h"

void TASProcessXInput(XINPUT_STATE* &pState);

bool TASProcessInput(__int64& a, InputEventType_t& nType, int& nTick, ButtonCode_t& scanCode, ButtonCode_t& virtualCode, int& data3);

void TASFrameHook();
