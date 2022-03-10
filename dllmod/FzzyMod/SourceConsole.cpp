#include "SourceConsole.h"
#include <iostream>

std::unique_ptr<SourceConsole> m_sourceConsole;
HookedVTableFunc<decltype(&IVEngineServer::VTable::SpewFunc), &IVEngineServer::VTable::SpewFunc> IVEngineServer_SpewFunc;

SourceConsole& SourceCon()
{
	return *m_sourceConsole;
}

#define WRAPPED_MEMBER(name)                                                                                           \
    MemberWrapper<decltype(&SourceConsole::##name), &SourceConsole::##name, decltype(&SourceCon), &SourceCon>::Call

SourceConsole::SourceConsole() : m_gameConsole("client.dll", "GameConsole004"), m_engineServer("engine.dll", "VEngineServer022"), m_engineClient("engine.dll", "VEngineClient013")
{
	RegisterConsoleCommand("toggleconsole", WRAPPED_MEMBER(ToggleConsoleCommand), "Show/hide the console",
		0);
	RegisterConsoleCommand("clear", WRAPPED_MEMBER(ClearConsoleCommand), "Clears the console", 0);
	RegisterConsoleCommand("togglenoclip", WRAPPED_MEMBER(ToggleNoclip), "Toggles noclip", 0);

	InitialiseSource();
}

IVEngineClient SourceConsole::GetEngineClient() {
	return *m_engineClient;
}

typedef void(*ConCommand_ConCommand)(ConCommand*, const char*, void (*)(const CCommand&), const char*, int, void*);
typedef int64_t(*EnableNoclip)(void*);
typedef int64_t(*DisableNoclip)(void*);
typedef void* (*UTIL_EntityByIndex)(int);

void SourceConsole::RegisterConsoleCommand(const char* name, void (*callback)(const CCommand&), const char* helpString, int flags) {
	ConCommand_ConCommand conCommand = ConCommand_ConCommand((uintptr_t)GetModuleHandle("engine.dll") + 0x415f60);

	ConCommand* newCommand = new ConCommand();
	conCommand(newCommand, name, callback, helpString, flags, nullptr);
}

void InvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line,
	uintptr_t pReserved)
{
	// Do nothing so that _vsnprintf_s returns an error instead of aborting
}

__int64 SpewFuncHook(IVEngineServer* engineServer, SpewType_t type, const char* format, va_list args)
{
	char pTempBuffer[5020];

	// There are some cases where Titanfall will pass an invalid format string to this function, causing a crash.
	// To avoid this, we setup a temporary invalid parameter handler which will just continue execution.
	_invalid_parameter_handler oldHandler = _set_thread_local_invalid_parameter_handler(InvalidParameterHandler);
	int val = _vsnprintf_s(pTempBuffer, sizeof(pTempBuffer) - 1, format, args);
	_set_thread_local_invalid_parameter_handler(oldHandler);

	if (val == -1)
	{
		m_sourceConsole->Print("Failed to call _vsnprintf_s for SpewFunc (format = ");
		m_sourceConsole->Print(format);
		m_sourceConsole->Print(")");
		return IVEngineServer_SpewFunc(engineServer, type, format, args);
	}

	if (type == SPEW_MESSAGE)
	{
		m_sourceConsole->Print("SERVER (SPEW_MESSAGE): ");
		m_sourceConsole->Print(pTempBuffer);
	}
	else if (type == SPEW_WARNING)
	{
		m_sourceConsole->Print("SERVER (SPEW_WARNING): ");
		m_sourceConsole->Print(pTempBuffer);
	}
	else
	{
		m_sourceConsole->Print("SERVER (");
		m_sourceConsole->Print(std::to_string(type).c_str());
		m_sourceConsole->Print("): ");
		m_sourceConsole->Print(pTempBuffer);
	}

	return IVEngineServer_SpewFunc(engineServer, type, format, args);
}

void SourceConsole::InitialiseSource()
{
	m_gameConsole->Initialize();
	CConsoleDialog_OnCommandSubmitted.Hook(m_gameConsole->m_pConsole->m_vtable, WRAPPED_MEMBER(OnCommandSubmittedHook));
	IVEngineServer_SpewFunc.Hook(m_engineServer->m_vtable, SpewFuncHook);
}

void SourceConsole::OnCommandSubmittedHook(CConsoleDialog* consoleDialog, const char* pCommand)
{
	Print("] ");
	Print(pCommand);
	Print("\n");

	CConsoleDialog_OnCommandSubmitted(consoleDialog, pCommand);
}

void SourceConsole::ToggleNoclip(const CCommand& args) {
	int sv_cheats = *(int*)((uintptr_t)GetModuleHandle("engine.dll") + 0x12A50EEC);
	if (sv_cheats == 0) return;

	UTIL_EntityByIndex ebyindex = UTIL_EntityByIndex((uintptr_t)GetModuleHandle("server.dll") + 0x271180);

	void* player = ebyindex(1);
	if (player != nullptr) {
		if (noclipEnabled) {
			DisableNoclip dnoclip = DisableNoclip((uintptr_t)GetModuleHandle("server.dll") + 0x10E8B0);
			dnoclip(player);
			noclipEnabled = false;
		}
		else {
			EnableNoclip enoclip = EnableNoclip((uintptr_t)GetModuleHandle("server.dll") + 0x10E9C0);
			enoclip(player);
			noclipEnabled = true;
		}
	}
}

void SourceConsole::ToggleConsoleCommand(const CCommand& args)
{
	if (!m_gameConsole->m_bInitialized)
	{
		return;
	}

	if (!m_gameConsole->IsConsoleVisible())
	{
		m_gameConsole->Activate();
	}
	else
	{
		m_gameConsole->Hide();
	}
}

void SourceConsole::ClearConsoleCommand(const CCommand& args)
{
	if (!m_gameConsole->m_bInitialized)
	{
		return;
	}

	m_gameConsole->Clear();
}

void SourceConsole::ColorPrint(const SourceColor& clr, const char* pMessage)
{
	if (!m_gameConsole->m_bInitialized)
	{
		return;
	}

	m_gameConsole->m_pConsole->m_pConsolePanel->ColorPrint(clr, pMessage);
}

void SourceConsole::Print(const char* pMessage)
{
	if (!m_gameConsole->m_bInitialized)
	{
		return;
	}

	m_gameConsole->m_pConsole->m_pConsolePanel->Print(pMessage);
}

void SourceConsole::DPrint(const char* pMessage)
{
	if (!m_gameConsole->m_bInitialized)
	{
		return;
	}

	m_gameConsole->m_pConsole->m_pConsolePanel->DPrint(pMessage);
}