#pragma once
#include <cstdint>
#include "SourceInterface.h"
#include "VTableHooking.h"
#include <memory>
#include "IVEngineServer.h"

class ConCommand
{
    unsigned char unknown[0x68];

public:
    virtual void EngineDestructor(void)
    {
    }
    virtual bool IsCommand(void) const
    {
        return false;
    }
    virtual bool IsFlagSet(int flag)
    {
        return false;
    }
    virtual void AddFlags(int flags)
    {
    }
    virtual void RemoveFlags(int flags)
    {
    }
    virtual int GetFlags() const
    {
        return 0;
    }
    virtual const char* GetName(void) const
    {
        return nullptr;
    }
    virtual const char* GetHelpText(void) const
    {
        return nullptr;
    }
    virtual bool IsRegistered(void) const
    {
        return false;
    }
    // NOTE: there are more virtual methods here
    // NOTE: Not using the engine's destructor here because it doesn't do anything useful for us
};

// From Source SDK
class CCommand
{
public:
    CCommand() = delete;

    int64_t ArgC() const;
    const char** ArgV() const;
    const char* ArgS() const;                 // All args that occur after the 0th arg, in string form
    const char* GetCommandString() const;     // The entire command in string form, including the 0th arg
    const char* operator[](int nIndex) const; // Gets at arguments
    const char* Arg(int nIndex) const;        // Gets at arguments

    static int MaxCommandLength();

private:
    enum
    {
        COMMAND_MAX_ARGC = 64,
        COMMAND_MAX_LENGTH = 512,
    };

    int64_t m_nArgc;
    int64_t m_nArgv0Size;
    char m_pArgSBuffer[COMMAND_MAX_LENGTH];
    char m_pArgvBuffer[COMMAND_MAX_LENGTH];
    const char* m_ppArgv[COMMAND_MAX_ARGC];
};

inline int CCommand::MaxCommandLength()
{
    return COMMAND_MAX_LENGTH - 1;
}

inline int64_t CCommand::ArgC() const
{
    return m_nArgc;
}

inline const char** CCommand::ArgV() const
{
    return m_nArgc ? (const char**)m_ppArgv : NULL;
}

inline const char* CCommand::ArgS() const
{
    return m_nArgv0Size ? &m_pArgSBuffer[m_nArgv0Size] : "";
}

inline const char* CCommand::GetCommandString() const
{
    return m_nArgc ? m_pArgSBuffer : "";
}

inline const char* CCommand::Arg(int nIndex) const
{
    // FIXME: Many command handlers appear to not be particularly careful
    // about checking for valid argc range. For now, we're going to
    // do the extra check and return an empty string if it's out of range
    if (nIndex < 0 || nIndex >= m_nArgc)
        return "";
    return m_ppArgv[nIndex];
}

inline const char* CCommand::operator[](int nIndex) const
{
    return Arg(nIndex);
}

class EditablePanel
{
public:
    virtual ~EditablePanel() = 0;
    unsigned char unknown[0x2B0];
};

struct SourceColor
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
    unsigned char A;

    SourceColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    {
        R = r;
        G = g;
        B = b;
        A = a;
    }

    SourceColor()
    {
        R = 0;
        G = 0;
        B = 0;
        A = 0;
    }
};

class IConsoleDisplayFunc
{
public:
    virtual void ColorPrint(const SourceColor& clr, const char* pMessage) = 0;
    virtual void Print(const char* pMessage) = 0;
    virtual void DPrint(const char* pMessage) = 0;
};

class CConsolePanel : public EditablePanel, public IConsoleDisplayFunc
{
};

class CConsoleDialog
{
public:
    struct VTable
    {
        void* unknown[298];
        void (*OnCommandSubmitted)(CConsoleDialog* consoleDialog, const char* pCommand);
    };

    VTable* m_vtable;
    unsigned char unknown[0x398];
    CConsolePanel* m_pConsolePanel;
};

class CGameConsole
{
public:
    virtual ~CGameConsole() = 0;

    // activates the console, makes it visible and brings it to the foreground
    virtual void Activate() = 0;

    virtual void Initialize() = 0;

    // hides the console
    virtual void Hide() = 0;

    // clears the console
    virtual void Clear() = 0;

    // return true if the console has focus
    virtual bool IsConsoleVisible() = 0;

    virtual void SetParent(int parent) = 0;

    bool m_bInitialized;
    CConsoleDialog* m_pConsole;
};

class SourceConsole
{
public:

    SourceConsole();
    void InitialiseSource();
    void RegisterConsoleCommand(const char*, void (*callback)(const CCommand&), const char*, int);
        
    void ToggleConsoleCommand(const CCommand& args);
    void ClearConsoleCommand(const CCommand& args);

    void ToggleNoclip(const CCommand& args);

    void OnCommandSubmittedHook(CConsoleDialog* consoleDialog, const char* pCommand);

    void ColorPrint(const SourceColor& clr, const char* pMessage);
    void Print(const char* pMessage);
    void DPrint(const char* pMessage);

private:
    SourceInterface<CGameConsole> m_gameConsole;
    SourceInterface<IVEngineServer> m_engineServer;
    HookedVTableFunc<decltype(&CConsoleDialog::VTable::OnCommandSubmitted), &CConsoleDialog::VTable::OnCommandSubmitted>
        CConsoleDialog_OnCommandSubmitted;

    bool noclipEnabled;
};

extern HookedVTableFunc<decltype(&IVEngineServer::VTable::SpewFunc), &IVEngineServer::VTable::SpewFunc> IVEngineServer_SpewFunc;

__int64 SpewFuncHook(IVEngineServer*, SpewType_t, const char*, va_list);

extern std::unique_ptr<SourceConsole> m_sourceConsole;