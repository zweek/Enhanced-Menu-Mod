#pragma once
#include <Xinput.h>

struct handle_data {
	unsigned long process_id;
	HWND best_handle;
};

void TASProcessXInput(XINPUT_STATE* pState);

bool TASProcessInputDown(WPARAM& key);
bool TASProcessInputUp(WPARAM& key);

void hookDirectXPresent();
void setMovementHooks();
