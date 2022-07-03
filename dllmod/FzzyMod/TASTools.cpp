#pragma comment(lib,"d3d11.lib")
#include <Windows.h>
#include "TASTools.h"
#include "TF2Binds.h"
#include <chrono>
#include <iostream>
#include "include/MinHook.h"
#include "InputHooker.h"
#include "SourceConsole.h"
#include <string>

using namespace std;

typedef void(__fastcall* FRICTIONHOOK)();
static FRICTIONHOOK hookedFriction = nullptr;

template <typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
{
	return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

auto flipTimestamp = std::chrono::steady_clock::now();
bool flip;

bool forwardPressed;
bool backPressed;
bool rightPressed;
bool leftPressed;
bool crouchPressed;

bool lurchPressed;
bool bumpLaunchPressed;
bool surfStopPressed;

bool isLurching;

__int64 lastA;
int lastTick;

int spoofingJump;
int spoofingCrouch;

int spoofingForward;
int spoofingBack;
int spoofingRight;
int spoofingLeft;

float velocity;
float oldvelocity;

bool isBumpLaunching;

int pressJump;

bool doKick;

void TASFrameHook() {
	__try {
		//float timescale = *(float*)((uintptr_t)GetModuleHandle("engine.dll") + 0x1315A2C8);
		//if (timescale >= 0.9f) return;
		float velX = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C2C);
		float velY = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C30);
		float velZ = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C34);
		float vel = (float)sqrtf(pow(velX, 2) + pow(velY, 2) + pow(velZ, 2));
		oldvelocity = velocity;
		velocity = vel;

		if (bumpLaunchPressed) {
			if (isBumpLaunching) {
				spoofPostEvent(lastA, IE_ButtonReleased, lastTick, (ButtonCode_t)jumpBinds[0], (ButtonCode_t)jumpBinds[0], 0);
				spoofPostEvent(lastA, IE_ButtonReleased, lastTick, (ButtonCode_t)backBinds[0], (ButtonCode_t)backBinds[0], 0);
				isBumpLaunching = false;
			}
			else {
				spoofPostEvent(lastA, IE_ButtonPressed, lastTick, (ButtonCode_t)jumpBinds[0], (ButtonCode_t)jumpBinds[0], 0);
				spoofPostEvent(lastA, IE_ButtonPressed, lastTick, (ButtonCode_t)backBinds[0], (ButtonCode_t)backBinds[0], 0);
				isBumpLaunching = true;
			}
		}
		else if (isBumpLaunching) {
			spoofPostEvent(lastA, IE_ButtonReleased, lastTick, (ButtonCode_t)jumpBinds[0], (ButtonCode_t)jumpBinds[0], 0);
			spoofPostEvent(lastA, IE_ButtonReleased, lastTick, (ButtonCode_t)backBinds[0], (ButtonCode_t)backBinds[0], 0);
			isBumpLaunching = false;
		}

		if (lurchPressed) {
			if (isLurching) {
				spoofPostEvent(lastA, IE_ButtonReleased, lastTick, (ButtonCode_t)forwardBinds[0], (ButtonCode_t)forwardBinds[0], 0);
				isLurching = false;
			}
			else {
				spoofPostEvent(lastA, IE_ButtonPressed, lastTick, (ButtonCode_t)forwardBinds[0], (ButtonCode_t)forwardBinds[0], 0);
				isLurching = true;
			}
		}
		else if (isLurching) {
			spoofPostEvent(lastA, IE_ButtonReleased, lastTick, (ButtonCode_t)forwardBinds[0], (ButtonCode_t)forwardBinds[0], 0);
			isLurching = false;
		}

		if (pressJump > 0) {
			pressJump--;
			if (pressJump == 1) {
				spoofPostEvent(lastA, IE_ButtonPressed, lastTick, (ButtonCode_t)jumpBinds[0], (ButtonCode_t)jumpBinds[0], 0);
				spoofingJump = 2;
			}
		}

		if (spoofingJump > 0) {
			spoofingJump--;
			if (spoofingJump == 0) {
				spoofPostEvent(lastA, IE_ButtonReleased, lastTick, (ButtonCode_t)jumpBinds[0], (ButtonCode_t)jumpBinds[0], 0);
			}
		}

		if (spoofingCrouch > 0) {
			spoofingCrouch--;
			if (spoofingCrouch == 0) {
				spoofPostEvent(lastA, IE_ButtonReleased, lastTick, (ButtonCode_t)crouchBinds[0], (ButtonCode_t)crouchBinds[0], 0);
			}
		}

		if (spoofingForward > 0) {
			spoofingForward--;
			if (spoofingForward == 0) {
				spoofPostEvent(lastA, IE_ButtonReleased, lastTick, (ButtonCode_t)forwardBinds[0], (ButtonCode_t)forwardBinds[0], 0);
			}
		}

		if (spoofingBack > 0) {
			spoofingBack--;
			if (spoofingBack == 0) {
				spoofPostEvent(lastA, IE_ButtonReleased, lastTick, (ButtonCode_t)backBinds[0], (ButtonCode_t)backBinds[0], 0);
			}
		}

		if (spoofingRight > 0) {
			spoofingRight--;
			if (spoofingRight == 0) {
				spoofPostEvent(lastA, IE_ButtonReleased, lastTick, (ButtonCode_t)rightBinds[0], (ButtonCode_t)rightBinds[0], 0);
			}
		}

		if (spoofingLeft > 0) {
			spoofingLeft--;
			if (spoofingLeft == 0) {
				spoofPostEvent(lastA, IE_ButtonReleased, lastTick, (ButtonCode_t)leftBinds[0], (ButtonCode_t)leftBinds[0], 0);
			}
		}

		if (doKick) {
			doKick = false;
			if (spoofingJump == 0) {
				spoofPostEvent(lastA, IE_ButtonPressed, lastTick, (ButtonCode_t)jumpBinds[0], (ButtonCode_t)jumpBinds[0], 0);
				spoofingJump = 1;
			}
			float velZ = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C34);
			if (spoofingCrouch == 0 && !crouchPressed && velZ != 0) {
				spoofPostEvent(lastA, IE_ButtonPressed, lastTick, (ButtonCode_t)crouchBinds[0], (ButtonCode_t)crouchBinds[0], 0);
				spoofingCrouch = 1;
			}
		}
	}
	__except (filter(GetExceptionCode(), GetExceptionInformation())) {
	}
}

bool TASProcessInput(__int64& a, InputEventType_t& nType, int& nTick, ButtonCode_t& scanCode, ButtonCode_t& virtualCode, int& data3) {
	__try {
		lastA = a;
		lastTick = nTick;
		ButtonCode_t key = scanCode;
		//float timescale = *(float*)((uintptr_t)GetModuleHandle("engine.dll") + 0x1315A2C8);
		//if (timescale >= 0.9f) return false;
		if (nType == IE_ButtonPressed) {
			if (key == jumpBinds[0] || key == jumpBinds[1]) {
				pressJump = 2;
				return true;
			}
			if (key == tasBumpLaunchBinds[0] || key == tasBumpLaunchBinds[1]) {
				bumpLaunchPressed = true;
				return true;
			}
			if (key == tasSurfStopBinds[0] || key == tasSurfStopBinds[1]) {
				surfStopPressed = true;
				return true;
			}
			if (key == tasLurchBinds[0] || key == tasLurchBinds[1]) {
				lurchPressed = true;
				float velX = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C2C);
				float velY = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C30);

				float wishYaw = (float)((180 / 3.14159265f) * -atan2f(velX, velY)) + 90;

				float* yaw = (float*)((*(uintptr_t*)((uintptr_t)GetModuleHandle("client.dll") + 0xE69EA0)) + 0x1E94);

				*yaw = wishYaw;

				return true;
			}
			if (key == forwardBinds[0] || key == forwardBinds[1]) {
				forwardPressed = true;
				return true;
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

			if (key == crouchBinds[0] || key == crouchBinds[1]) crouchPressed = true;
		}
		if (nType == IE_ButtonReleased) {
			if (key == tasSurfStopBinds[0] || key == tasSurfStopBinds[1]) {
				surfStopPressed = false;
				return true;
			}
			if (key == tasBumpLaunchBinds[0] || key == tasBumpLaunchBinds[1]) {
				bumpLaunchPressed = false;
				return true;
			}
			if (key == tasLurchBinds[0] || key == tasLurchBinds[1]) {
				lurchPressed = false;
				return true;
			}
			if (key == forwardBinds[0] || key == forwardBinds[1]) {
				forwardPressed = false;
				return true;
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
			if (key == crouchBinds[0] || key == crouchBinds[1]) crouchPressed = false;
		}
	}
	__except (filter(GetExceptionCode(), GetExceptionInformation())) {
	}
	return false;
}

bool pressingLurch = false;

int packet = 0;

void TASProcessXInput(XINPUT_STATE*& pState) {
	__try {
		float timescale = *(float*)((uintptr_t)GetModuleHandle("engine.dll") + 0x1315A2C8);
		float velX = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C2C);
		float velY = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C30);
		float velZ = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C34);
		float velocity = (float)sqrtf(pow(velX, 2) + pow(velY, 2) + pow(velZ, 2));

		pState->dwPacketNumber = packet;
		if (packet == 0) packet = 1;
		else packet = 0;

		pState->Gamepad.sThumbLY = (short)0;
		if (forwardPressed) {
			if (!backPressed) {
				pState->Gamepad.sThumbLY = (short)32767;
			}
		}
		if (backPressed) {
			if (!forwardPressed) {
				pState->Gamepad.sThumbLY = (short)-32767;
			}
		}

		pState->Gamepad.sThumbLX = (short)0;
		if (pressJump == 0) {
			if (rightPressed) {
				if (!leftPressed && !lurchPressed && !spoofingRight) {
					pState->Gamepad.sThumbLX = (short)32767;

					if (timescale < 1.0f && !forwardPressed && !backPressed) {
						float margin = 10.0f;
						float airspeed = 60.0f;
						if (velZ != 0) {
							float offset = (float)((180 / 3.14159265f) * abs(asinf((airspeed - margin) / velocity)));
							float wishYaw = (float)((180 / 3.14159265f) * -atan2f(velX, velY)) + 90 + offset;

							float* yaw = (float*)((*(uintptr_t*)((uintptr_t)GetModuleHandle("client.dll") + 0xE69EA0)) + 0x1E94);

							*yaw = wishYaw;
						}
					}
				}
			}
			if (leftPressed) {
				if (!rightPressed && !lurchPressed && !spoofingLeft) {
					pState->Gamepad.sThumbLX = (short)-32767;

					if (timescale < 1.0f && !forwardPressed && !backPressed) {
						float margin = 10.0f;
						float airspeed = 60.0f;
						if (velZ != 0) {
							float offset = (float)((180 / 3.14159265f) * abs(asinf((airspeed - margin) / velocity)));
							float wishYaw = (float)((180 / 3.14159265f) * -atan2f(velX, velY)) + 90 - offset;

							float* yaw = (float*)((*(uintptr_t*)((uintptr_t)GetModuleHandle("client.dll") + 0xE69EA0)) + 0x1E94);

							*yaw = wishYaw;
						}
					}
				}
			}
		}

		if (timescale < 1.0f) {
			bool sprintPressed = *(bool*)((uintptr_t)GetModuleHandle("engine.dll") + 0x1396CAB8);
			if (velocity < oldvelocity - 0.1f && velocity > 300 && !sprintPressed) {
				bool onGround = *(bool*)((uintptr_t)GetModuleHandle("client.dll") + 0x11EED78);
				if (onGround) {
					doKick = true;
				}
			}
			if (doKick) {
				pState->Gamepad.sThumbLX = (short)0;
				pState->Gamepad.sThumbLY = (short)32767;
			}
		}
		oldvelocity = velocity;
	}
	__except (filter(GetExceptionCode(), GetExceptionInformation())) {
	}
	/*if (!forwardPressed && !backPressed && !rightPressed && !leftPressed) {
		float velX = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C2C);
		float velY = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C30);

		float yaw = *(float*)((uintptr_t)GetModuleHandle("engine.dll") + 0x7B6668);

		float toDegrees = (180.0f / 3.14159265f);
		float toRadians = (3.14159265f / 180.0f);
		float magnitude = sqrt(pow(velX, 2) + pow(velY, 2));

		auto elapsed = chrono::steady_clock::now() - flipTimestamp;
		long long since = chrono::duration_cast<chrono::milliseconds>(elapsed).count();
		if (since > 100) {
			//flip = !flip;
		}

		if (magnitude > 100) {
			float airSpeed = 20;
			float velDegrees = atan2f(velY, velX) * toDegrees;
			float velDirection = yaw - velDegrees;

			float rightx = sinf((velDirection + 90) * toRadians);
			float righty = cosf((velDirection + 90) * toRadians);

			float offsetx = velX + rightx * airSpeed;
			float offsety = velY + righty * airSpeed;

			float angle = atan2f(offsety, offsetx) - atan2f(velY, velX);
			float offsetAngle = atan2f(righty, rightx) - angle;

			short tx = cosf(offsetAngle * toRadians) * 32767.0f;
			short ty = sinf(offsetAngle * toRadians) * 32767.0f;

			pState->Gamepad.sThumbLX = tx;
			pState->Gamepad.sThumbLY = ty;
		}
	}*/
}