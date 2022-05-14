#pragma comment(lib,"d3d11.lib")
#include <Windows.h>
#include "TASTools.h"
#include "TF2Binds.h"
#include <chrono>
#include <iostream>
#include "include/MinHook.h"
#include "InputHooker.h"
#include "SourceConsole.h"

using namespace std;

auto flipTimestamp = std::chrono::steady_clock::now();
bool flip;

bool forwardPressed;
bool backPressed;
bool rightPressed;
bool leftPressed;

bool TASProcessInput(__int64 &a, InputEventType_t &nType, int &nTick, ButtonCode_t &scanCode, ButtonCode_t &virtualCode, int &data3) {
	ButtonCode_t key = scanCode;
	if (nType == IE_ButtonPressed) {
		if (key == forwardBinds[0] || key == forwardBinds[1]) {
			forwardPressed = true;
			nType = IE_AnalogValueChanged;
			scanCode = (ButtonCode_t)3;
			virtualCode = (ButtonCode_t)32000;
			data3 = (ButtonCode_t)32000;
		}
		if (key == backBinds[0] || key == backBinds[1]) {
			backPressed = true;
			return true;
		}
		if (key == rightBinds[0] || key == rightBinds[1]) {
			rightPressed = true;
			return true;
		}
		if (key == leftBinds[0] || key == leftBinds[1]) {
			leftPressed = true;
			return true;
		}
	}
	if (nType == IE_ButtonReleased) {
		if (key == forwardBinds[0] || key == forwardBinds[1]) {
			forwardPressed = false;
			nType = IE_AnalogValueChanged;
			scanCode = (ButtonCode_t)3;
			virtualCode = (ButtonCode_t)0;
			data3 = (ButtonCode_t)-32000;
		}
		if (key == backBinds[0] || key == backBinds[1]) {
			backPressed = false;
			return true;
		}
		if (key == rightBinds[0] || key == rightBinds[1]) {
			rightPressed = false;
			return true;
		}
		if (key == leftBinds[0] || key == leftBinds[1]) {
			leftPressed = false;
			return true;
		}
	}
	return false;
}

/*void TASProcessInputProc(UINT& uMsg, WPARAM& wParam, LPARAM& lParam) {
	bool onGround = *(bool*)((uintptr_t)GetModuleHandle("client.dll") + 0x11EED78);

	if (uMsg == WM_KEYDOWN) {
		//int scanCode = (lParam >> 16) & 0xFF;
		//cout << scanCode << endl;
		if (wParam == forwardBinds[0] || wParam == forwardBinds[1]) {
			simulateKeyDown(leftBinds[0]);
			//wParam = leftBinds[0];
			if (!onGround) {
				//uMsg = WM_NULL;
			}
		}
	}
}*/

bool pressingLurch = false;

void TASProcessXInput(XINPUT_STATE* &pState) {
	if (forwardPressed) {
		if (!backPressed) {
			pState->Gamepad.sThumbLY = (short)32767;
			pState->Gamepad.sThumbLX = (short)32767;
		}
	}
	if (backPressed) {
		if (!forwardPressed) {
			pState->Gamepad.sThumbLY = (short)-32767;
			pState->Gamepad.sThumbLX = (short)-32767;
		}
	}
	if (rightPressed) {
		if (!leftPressed) pState->Gamepad.sThumbLX = (short)32767;
	}
	if (leftPressed) {
		if (!rightPressed) pState->Gamepad.sThumbLX = (short)-32767;
	}
	/*float velX = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C2C);
	float velY = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C30);

	float yaw = *(float*)((uintptr_t)GetModuleHandle("engine.dll") + 0x7B6668);

	float toDegrees = (180.0f / 3.14159265f);
	float toRadians = (3.14159265f / 180.0f);
	float magnitude = sqrt(pow(velX, 2) + pow(velY, 2));

	auto elapsed = chrono::steady_clock::now() - flipTimestamp;
	long long since = chrono::duration_cast<chrono::milliseconds>(elapsed).count();
	if (since > 100) {
		flip = !flip;
	}

	if (magnitude > 100) {
		float airSpeed = 50;
		float velDegrees = atan2f(velY, velX) * toDegrees;
		float velDirection = yaw - velDegrees;

		float rightx = sinf((velDirection + 90) * toRadians);
		float righty = cosf((velDirection + 90) * toRadians);

		float offsetx = velX + rightx * airSpeed;
		float offsety = velY + righty * airSpeed;

		float angle = atan2f(offsety, offsetx) - atan2f(velY, velX);
		float offsetAngle = atan2f(righty, rightx) - angle;
		cout << offsetAngle << endl;

		short tx = cosf(offsetAngle * toRadians) * 32767.0f;
		short ty = sinf(offsetAngle * toRadians) * 32767.0f;
		//cout << tx << " : " << ty << endl;

		pState->Gamepad.sThumbLX = tx;
		pState->Gamepad.sThumbLY = ty;
	}*/
}