#include "Speedmod.h"
#include <Windows.h>
#include <initguid.h>
#include <KnownFolders.h>
#include <urlmon.h>
#include <iostream>
#include <ShlObj.h>
#include <sstream>
#include <chrono>
#include "SourceConsole.h"
#include "Util.h"
#include "InputHooker.h"
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "shell32.lib")

using namespace std;

bool speedmodWasEnabled;
bool wasLoading;

string delayedLoadSave;
int delayedLoadMillis;

auto lastTimestamp = chrono::steady_clock::now();

bool fileExists(const string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

void MakeAlliesInvincible() {
    uintptr_t pos1 = (uintptr_t)GetModuleHandle("server.dll") + 0x43373A;
    Util::WriteBytes(pos1, { 0x83, 0xBB, 0x10, 0x01, 0x00, 0x00, 0x03, 0x74, 0x02, 0x89, 0x3B, 0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48,
                0x83, 0xC4, 0x20, 0x5F, 0xC3 });
    uintptr_t pos2 = (uintptr_t)GetModuleHandle("server.dll") + 0x433725;
    Util::WriteBytes(pos2, { 0x74, 0x1E });
}

void MakeAlliesKillable() {
    uintptr_t pos1 = (uintptr_t)GetModuleHandle("server.dll") + 0x43373A;
    Util::WriteBytes(pos1, {
                0x89, 0x3B, 0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48, 0x83, 0xC4, 0x20, 0x5F, 0xC3, 0xCC, 0xCC, 0xCC, 0xCC,
                0xCC, 0xCC, 0xCC, 0xCC, 0xCC });
    uintptr_t pos2 = (uintptr_t)GetModuleHandle("server.dll") + 0x433725;
    Util::WriteBytes(pos2, { 0x74, 0x15 });
}

void RemoveWallFriction() {
    uintptr_t pos1 = (uintptr_t)GetModuleHandle("client.dll") + 0x20D6E5;
    uintptr_t pos2 = (uintptr_t)GetModuleHandle("server.dll") + 0x185D36;
    std::vector<int> code = {
                0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
    };
    Util::WriteBytes(pos1, code);
    Util::WriteBytes(pos2, code);
}

void RestoreWallFriction() {
    uintptr_t pos1 = (uintptr_t)GetModuleHandle("client.dll") + 0x20D6E5;
    uintptr_t pos2 = (uintptr_t)GetModuleHandle("server.dll") + 0x185D36;
    std::vector<int> code = {
                0xF3, 0x0F, 0x11, 0x81, 0x8C, 0x00, 0x00, 0x00, // movss [rcx+8C],xmm0
                0xF3, 0x0F, 0x59, 0x89, 0x90, 0x00, 0x00, 0x00, // mulss xmm1,[rcx+90]
                0xF3, 0x0F, 0x11, 0x89, 0x90, 0x00, 0x00, 0x00, // movss [rcx+90],xmm1
    };
    Util::WriteBytes(pos1, code);
    Util::WriteBytes(pos2, code);
}

void InstallSpeedmodSaves() {
    CHAR documents[MAX_PATH];
    HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, documents);
    if (result == S_OK) {
        stringstream savegames;
        savegames << documents << "\\Respawn\\Titanfall2\\profile\\savegames";
        stringstream ss;
        ss << documents << "\\Respawn\\Titanfall2\\profile\\savegames\\installspeedmodsaves.exe";
        bool allExist = true;
        for (int i = 1; i <= 12; i++) {
            stringstream save;
            save << savegames.str() << "\\speedmod" << i << ".sav";
            bool exists = fileExists(save.str());
            if (!exists) {
                allExist = false;
                break;
            }
        }
        if (allExist) return;

        HRESULT hr = URLDownloadToFile(NULL, "https://github.com/Fzzy2j/FzzySplitter/releases/download/v1.0/installspeedmodsaves.exe", 
            (LPCSTR)(ss.str().c_str()), 0, NULL);
        cout << "download speedmod complete" << endl;

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        CreateProcess(ss.str().c_str(),
            NULL,
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            savegames.str().c_str(),
            &si,
            &pi
        );

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        cout << "getfolderpath failed" << endl;
    }

    //CoTaskMemFree(static_cast<void*>(documents));
}

void LoadSave(string save, int delay) {
    if (delayedLoadMillis > 0) return;
    delayedLoadMillis = delay;
    delayedLoadSave = save;
}

void LoadSave(string save) {
    stringstream cmd;
    cmd << "load " << save.c_str() << "; set_loading_progress_detente #INTROSCREEN_HINT_PC #INTROSCREEN_HINT_CONSOLE";
    auto engineClient = m_sourceConsole->GetEngineClient();
    engineClient.m_vtable->ClientCmd_Unrestricted(&engineClient, cmd.str().c_str());
}

void SpeedmodTick() {
    bool speedmodEnabled = Util::SRMM_GetSetting(SRMM_ENABLE_SPEEDMOD);
    if (!speedmodWasEnabled && speedmodEnabled) {
        InstallSpeedmodSaves();
        MakeAlliesInvincible();
        RemoveWallFriction();
    }
    if (speedmodWasEnabled && !speedmodEnabled) {
        MakeAlliesKillable();
        RestoreWallFriction();

        float* airAccel = (float*)Util::FindDMAAddy((uintptr_t)GetModuleHandle("engine.dll") + 0x13084248, { 0x2564 });
        float* airSpeed = (float*)Util::FindDMAAddy((uintptr_t)GetModuleHandle("engine.dll") + 0x13084248, { 0xEA8, 0x1008, 0x1038, 0x390, 0x48, 0x18, 0xA30, 0x10, 0x2218 });
        float* lurchMax = (float*)Util::FindDMAAddy((uintptr_t)GetModuleHandle("client.dll") + 0x11B0308, {  });
        float* slideStepVelocityReduction = (float*)Util::FindDMAAddy((uintptr_t)GetModuleHandle("client.dll") + 0x11B0D28, {  });
        float* slideBoostCooldown = (float*)Util::FindDMAAddy((uintptr_t)GetModuleHandle("client.dll") + 0x11B3AD8, {  });
        *airAccel = 500.0f;
        *airSpeed = 60.0f;
        *lurchMax = 0.7f;
        *slideStepVelocityReduction = 10.0f;
        *slideBoostCooldown = 2.0f;
    }
    speedmodWasEnabled = speedmodEnabled;
    if (!speedmodEnabled) return;

    /*auto t = chrono::steady_clock::now() - lastTimestamp;
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(t).count();
    lastTimestamp = std::chrono::steady_clock::now();
    if (delayedLoadMillis > 0) {
        delayedLoadMillis -= elapsed;
        if (delayedLoadMillis <= 0) {
            LoadSave(delayedLoadSave);
        }
    }*/

    int clFrames = *(int*)(*(uintptr_t*)((uintptr_t)GetModuleHandle("materialsystem_dx11.dll") + 0x1A9F4A8) + 0x58C);
    bool inLoadingScreen = *(bool*)((uintptr_t)GetModuleHandle("client.dll") + 0xB38C5C);
    int tickCount = *(int*)((uintptr_t)GetModuleHandle("engine.dll") + 0x765A24);
    bool isLoading = clFrames <= 0 || inLoadingScreen || tickCount <= 23;

    float* airAccel = (float*)Util::FindDMAAddy((uintptr_t)GetModuleHandle("engine.dll") + 0x13084248, { 0x2564 });
    float* airSpeed = (float*)Util::FindDMAAddy((uintptr_t)GetModuleHandle("engine.dll") + 0x13084248, { 0xEA8, 0x1008, 0x1038, 0x390, 0x48, 0x18, 0xA30, 0x10, 0x2218 });
    float* lurchMax = (float*)Util::FindDMAAddy((uintptr_t)GetModuleHandle("client.dll") + 0x11B0308, {  });
    float* slideStepVelocityReduction = (float*)Util::FindDMAAddy((uintptr_t)GetModuleHandle("client.dll") + 0x11B0D28, {  });
    float* slideBoostCooldown = (float*)Util::FindDMAAddy((uintptr_t)GetModuleHandle("client.dll") + 0x11B3AD8, {  });

    if (!isLoading) {
        *airAccel = 10000.0f;
        *airSpeed = 40.0f;
        *lurchMax = 0.0f;
        *slideStepVelocityReduction = 0;
        *slideBoostCooldown = 0.0f;
    }

    wasLoading = isLoading;
}