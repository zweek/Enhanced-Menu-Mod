#pragma once

class IVEngineClient
{
public:
    struct VTable
    {
        void* unknown[279];
        void (*ClientCmd_Unrestricted)(IVEngineClient* engineClient, const char* szCmdString);
    };

    VTable* m_vtable;
};