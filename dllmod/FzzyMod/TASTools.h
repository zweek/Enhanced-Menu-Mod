#pragma once
#include <Xinput.h>

struct handle_data {
	unsigned long process_id;
	HWND best_handle;
};

void TASProcessXInput(XINPUT_STATE* pState);

void TASProcessInputProc(UINT &uMsg, WPARAM &wParam, LPARAM &lParam);

void hookDirectXPresent();
